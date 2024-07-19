#include "BYTETracker.h"
#include <fstream>
#include <chrono>

using namespace std;

//***********************************************************************

static std::vector<std::vector<float>> all_bbox;


static float calculate_iou(const std::vector<float>& rect1, const std::vector<float>& rect2) {
    float overlap_x = std::max(0.0f, std::min(rect1[3], rect2[3]) - std::max(rect1[1], rect2[1]));
    float overlap_y = std::max(0.0f, std::min(rect1[4], rect2[4]) - std::max(rect1[2], rect2[2]));
    float overlap_area = overlap_x * overlap_y;
    float total_area = ((rect1[3] - rect1[1]) * (rect1[4] - rect1[2])) + ((rect2[3] - rect2[1]) * (rect2[4] - rect2[2])) - overlap_area;
    return overlap_area / total_area;
}


//**************************************************************************

//static long long get_microseconds(void) {
//    // 微秒单位
//    struct timespec now;
//    clock_gettime(CLOCK_MONOTONIC, &now);
//    return now.tv_sec * 1000000 + now.tv_nsec / 1000;
//}

BYTETracker::BYTETracker(int frame_rate, int track_buffer)
{
    track_thresh = 0.45;
    high_thresh = 0.5;
    match_thresh = 0.80;

    new_stracks_thresh_ped = 0.1;
    max_ped_area = 400;
    max_supplementary_box_count = 2;

    frame_id = 0;
    max_time_lost = int(frame_rate / 30.0 * track_buffer);

    img_w_ = 640;
    img_h_ = 512;

    //    cout << "Init ByteTrack!" << endl;
}

BYTETracker::~BYTETracker()
{
}

void BYTETracker::update(const vector<Object>& objects)
{

    ////////////////// Step 1: Get detections //////////////////
    this->frame_id++;
    vector<STrack> activated_stracks;
    vector<STrack> refind_stracks;
    vector<STrack> removed_stracks;
    vector<STrack> lost_stracks;
    vector<STrack> detections;
    vector<STrack> detections_low;

    vector<STrack> detections_cp;
    vector<STrack> tracked_stracks_swap;
    vector<STrack> resa, resb;
    //    vector<STrack> output_stracks;

    vector<STrack>().swap(this->output_stracks);

    vector<STrack*> unconfirmed;
    vector<STrack*> tracked_stracks;
    vector<STrack*> strack_pool;
    vector<STrack*> r_tracked_stracks;

    vector<STrack> strack_pool_cp;

    if (objects.size() > 0)
    {
        for (int i = 0; i < objects.size(); i++)
        {
            vector<float> tlbr_;
            tlbr_.resize(4);
            tlbr_[0] = objects[i].rect.x;
            tlbr_[1] = objects[i].rect.y;
            tlbr_[2] = objects[i].rect.x + objects[i].rect.width;
            tlbr_[3] = objects[i].rect.y + objects[i].rect.height;

            float score = objects[i].prob;
            int cls_id = objects[i].label;

            if (cls_id == 1) {
                cout << "" << endl;
            }

            STrack strack(STrack::tlbr_to_tlwh(tlbr_), score, cls_id);
            if (score >= track_thresh)
            {
                detections.push_back(strack);
            }
            else if (cls_id == 2 && score > 0.1 && tlbr_[2] * tlbr_[3] < this->max_ped_area)
            {
                detections.push_back(strack);
            }
            else
            {
                detections_low.push_back(strack);
            }

        }
    }

    // Add newly detected tracklets to tracked_stracks
    for (int i = 0; i < this->tracked_stracks.size(); i++)
    {
        if (!this->tracked_stracks[i].is_activated)
            unconfirmed.push_back(&this->tracked_stracks[i]);
        else
            tracked_stracks.push_back(&this->tracked_stracks[i]);
    }

    ////////////////// Step 2: First association, with IoU //////////////////
    strack_pool = joint_stracks(tracked_stracks, this->lost_stracks);

    for (int i = 0; i < strack_pool.size(); i++) {
        strack_pool_cp.push_back(*strack_pool[i]);
    }
    STrack::multi_predict(strack_pool, this->kalman_filter);

    vector<vector<float> > dists;
    int dist_size = 0, dist_size_size = 0;


    dists = iou_distance(strack_pool, detections, dist_size, dist_size_size);


    vector<vector<int> > matches;
    vector<int> u_track, u_detection;
    linear_assignment(dists, dist_size, dist_size_size, match_thresh, matches, u_track, u_detection);


    for (int i = 0; i < matches.size(); i++)
    {
        STrack* track = strack_pool[matches[i][0]];
        STrack* det = &detections[matches[i][1]];
        if (track->state == TrackState::Tracked)
        {
            track->update(*det, this->frame_id);
            activated_stracks.push_back(*track);
        }
        else
        {
            track->re_activate(*det, this->frame_id, false);
            refind_stracks.push_back(*track);
        }
        track->supplementary_box_count = 0;
    }

    ////////////////// Step 3: Second association, using low score dets //////////////////
    for (int i = 0; i < u_detection.size(); i++)
    {
        detections_cp.push_back(detections[u_detection[i]]);
    }
    detections.clear();
    detections.assign(detections_low.begin(), detections_low.end());

    for (int i = 0; i < u_track.size(); i++)
    {
        if (strack_pool[u_track[i]]->state == TrackState::Tracked)
        {
            r_tracked_stracks.push_back(strack_pool[u_track[i]]);
        }
    }

    dists.clear();
    dists = iou_distance(r_tracked_stracks, detections, dist_size, dist_size_size);

    matches.clear();
    u_track.clear();
    u_detection.clear();
    linear_assignment(dists, dist_size, dist_size_size, 0.5, matches, u_track, u_detection);

    for (int i = 0; i < matches.size(); i++)
    {
        STrack* track = r_tracked_stracks[matches[i][0]];
        STrack* det = &detections[matches[i][1]];
        if (track->state == TrackState::Tracked)
        {
            track->update(*det, this->frame_id);
            activated_stracks.push_back(*track);
        }
        else
        {
            track->re_activate(*det, this->frame_id, false);
            refind_stracks.push_back(*track);
        }
        track->supplementary_box_count = 0;
    }

    for (int i = 0; i < u_track.size(); i++)
    {
        STrack* track = r_tracked_stracks[u_track[i]];
        if (track->cls_id == 2) {
            if (track->tlwh[2] * track->tlwh[3] < this->max_ped_area) {
                if (track->tracklet_len >= 1 && track->supplementary_box_count < track->tracklet_len && track->supplementary_box_count < this->max_supplementary_box_count) {
                    STrack supplyment_det(track->tlwh, track->score, track->cls_id);
                    track->update(supplyment_det, this->frame_id);
                    int add_count = 1;
                    for (int j = 0; j < strack_pool_cp.size(); j++) {
                        if (track->track_id == strack_pool_cp[j].track_id) {
                            float xmin = std::max(track->tlbr[0], strack_pool_cp[j].tlbr[0]);
                            float ymin = std::max(track->tlbr[1], strack_pool_cp[j].tlbr[1]);
                            float xmax = std::min(track->tlbr[2], strack_pool_cp[j].tlbr[2]);
                            float ymax = std::min(track->tlbr[3], strack_pool_cp[j].tlbr[3]);
                            float inter_area = (xmax - xmin) * (ymax - ymin);
                            float iou_pre = inter_area / (track->tlwh[2] * track->tlwh[3] + strack_pool_cp[j].tlwh[2] * strack_pool_cp[j].tlwh[3] - inter_area);
                            if (iou_pre < 0.851) {
                                add_count = 2;
                                break;
                            }
                        }
                    }
                    strack_pool_cp.clear();
                    track->supplementary_box_count += add_count;
                    track->tracklet_len -= 1;
                    //track->state = TrackState::New;
                    activated_stracks.push_back(*track);
                    continue;
                }
            }
        }
        if (track->state != TrackState::Lost)
        {
            track->mark_lost();
            lost_stracks.push_back(*track);
        }
    }

    // Deal with unconfirmed tracks, usually tracks with only one beginning frame
    detections.clear();
    detections.assign(detections_cp.begin(), detections_cp.end());

    dists.clear();
    dists = iou_distance(unconfirmed, detections, dist_size, dist_size_size);

    matches.clear();
    vector<int> u_unconfirmed;
    u_detection.clear();
    linear_assignment(dists, dist_size, dist_size_size, 0.7, matches, u_unconfirmed, u_detection);

    for (int i = 0; i < matches.size(); i++)
    {
        unconfirmed[matches[i][0]]->update(detections[matches[i][1]], this->frame_id);
        activated_stracks.push_back(*unconfirmed[matches[i][0]]);
    }

    for (int i = 0; i < u_unconfirmed.size(); i++)
    {
        STrack* track = unconfirmed[u_unconfirmed[i]];
        track->mark_removed();
        removed_stracks.push_back(*track);
    }

    ////////////////// Step 4: Init new stracks //////////////////
    for (int i = 0; i < u_detection.size(); i++)
    {
        STrack* track = &detections[u_detection[i]];
        if (track->cls_id == 2) {
            if (track->score < this->new_stracks_thresh_ped)
                continue;
            if (track->tlwh[2] * track->tlwh[3] >= this->max_ped_area && track->score < this->high_thresh)
                continue;
        }
        else {
            if (track->score < this->high_thresh)
                continue;
        }

        track->activate(this->kalman_filter, this->frame_id);
        activated_stracks.push_back(*track);
    }

    ////////////////// Step 5: Update state //////////////////
    for (int i = 0; i < this->lost_stracks.size(); i++)
    {
        if (this->frame_id - this->lost_stracks[i].end_frame() > this->max_time_lost)
        {
            this->lost_stracks[i].mark_removed();
            removed_stracks.push_back(this->lost_stracks[i]);
        }
    }

    for (int i = 0; i < this->tracked_stracks.size(); i++)
    {
        if (this->tracked_stracks[i].state == TrackState::Tracked)
        {
            tracked_stracks_swap.push_back(this->tracked_stracks[i]);
        }
    }
    this->tracked_stracks.clear();
    this->tracked_stracks.assign(tracked_stracks_swap.begin(), tracked_stracks_swap.end());

    this->tracked_stracks = joint_stracks(this->tracked_stracks, activated_stracks);
    this->tracked_stracks = joint_stracks(this->tracked_stracks, refind_stracks);

    //std::cout << activated_stracks.size() << std::endl;

    this->lost_stracks = sub_stracks(this->lost_stracks, this->tracked_stracks);
    for (int i = 0; i < lost_stracks.size(); i++)
    {
        this->lost_stracks.push_back(lost_stracks[i]);
    }

    this->lost_stracks = sub_stracks(this->lost_stracks, this->removed_stracks);
    remove_duplicate_stracks(resa, resb, this->tracked_stracks, this->lost_stracks);
    // clip remove stracks to 50 maximum
    if (this->removed_stracks.size() > 100) {
        this->removed_stracks.erase(this->removed_stracks.begin(), this->removed_stracks.end() - 49);
    }
    for (int i = 0; i < removed_stracks.size(); i++)
    {
        this->removed_stracks.push_back(removed_stracks[i]);
    }

    this->tracked_stracks.clear();
    this->tracked_stracks.assign(resa.begin(), resa.end());
    this->lost_stracks.clear();
    this->lost_stracks.assign(resb.begin(), resb.end());


    for (int i = 0; i < this->tracked_stracks.size(); i++)
    {
        if (this->tracked_stracks[i].is_activated)
        {

            float x1 = this->tracked_stracks[i].tlwh[0];
            float y1 = this->tracked_stracks[i].tlwh[1];

            float x2 = this->tracked_stracks[i].tlwh[0] + this->tracked_stracks[i].tlwh[2];
            float y2 = this->tracked_stracks[i].tlwh[1] + this->tracked_stracks[i].tlwh[3];

            x1 = std::max(std::min(x1, (float)(this->img_w_ - 1)), 0.f);
            y1 = std::max(std::min(y1, (float)(this->img_h_ - 1)), 0.f);

            x2 = std::max(std::min(x2, (float)(this->img_w_ - 1)), 0.f);
            y2 = std::max(std::min(y2, (float)(this->img_h_ - 1)), 0.f);

            if (x1 >= x2) {
                continue;
            }
            if (y1 >= y2) {
                continue;
            }

            this->tracked_stracks[i].tlwh[0] = x1;
            this->tracked_stracks[i].tlwh[1] = y1;
            this->tracked_stracks[i].tlwh[2] = x2 - x1;
            this->tracked_stracks[i].tlwh[3] = y2 - y1;
           
            this->output_stracks.push_back(this->tracked_stracks[i]);
        }
    }


    //**************************************************************************

    std::vector<int> a_idx;
    for (int i = 0; i < all_bbox.size(); i++)
    {
        int q = 0;
        for (int j = 0; j < this->output_stracks.size(); j++)
        {
            if (all_bbox[i][0] == this->output_stracks[j].track_id)
            {
                q = 1;
            }
        }
        if (q == 0)
        {
            a_idx.push_back(i);
        }
    }


    // 对索引进行排序，从大到小，避免索引问题
    std::sort(a_idx.begin(), a_idx.end(), std::greater<int>());

    // 删除指定索引位置的元素
    for (int index : a_idx) {
        all_bbox.erase(all_bbox.begin() + index);
    }



    for (int i = 0; i < this->output_stracks.size(); i++)
    {
    std:vector<float> tmp_box;

        float x1 = this->output_stracks[i].tlwh[0];
        float y1 = this->output_stracks[i].tlwh[1];

        float x2 = this->output_stracks[i].tlwh[0] + this->output_stracks[i].tlwh[2];
        float y2 = this->output_stracks[i].tlwh[1] + this->output_stracks[i].tlwh[3];

        tmp_box.push_back(output_stracks[i].track_id);
        tmp_box.push_back(x1);
        tmp_box.push_back(y1);
        tmp_box.push_back(x2);
        tmp_box.push_back(y2);
        tmp_box.push_back(this->output_stracks[i].cls_id);
        tmp_box.push_back(0);
        tmp_box.push_back(0);

        int p = 0;
        for (int j = 0; j < all_bbox.size(); j++)
        {
            if (all_bbox[j][0] == this->output_stracks[i].track_id)
            {
                //**************************************************
                if (all_bbox[j][5] == tmp_box[5])
                {
                    all_bbox[j][6] = all_bbox[j][6] + 1;
                    all_bbox[j][7] = 0;
                }
                else
                {
                    all_bbox[j][7] = all_bbox[j][7] + 1;
                    if (all_bbox[j][6] >= 5)
                    {
                        if (all_bbox[j][7] <= 5)
                        {
                            tmp_box[5] = all_bbox[j][5];
                        }
                        else
                        {
                            all_bbox[j][5] = tmp_box[5];
                            all_bbox[j][6] = 0;
                            all_bbox[j][7] = 0;
                        }
                    }
                    else
                    {
                        all_bbox[j][6] = 0;
                    }
                }
                tmp_box[6] = all_bbox[j][6];
                tmp_box[7] = all_bbox[j][7];

                //***************************************************


                float iou_result = calculate_iou(all_bbox[j], tmp_box);

                if (iou_result < 0.95)
                {
                    all_bbox[j][0] = tmp_box[0];
                    all_bbox[j][1] = tmp_box[1];
                    all_bbox[j][2] = tmp_box[2];
                    all_bbox[j][3] = tmp_box[3];
                    all_bbox[j][4] = tmp_box[4];
                    all_bbox[j][5] = tmp_box[5];
                    all_bbox[j][6] = tmp_box[6];
                    all_bbox[j][7] = tmp_box[7];
                    /*
                    std::vector<float> tmp_;
                    float t_idx = tmp_box[0];
                    float x11 = 0.5*tmp_box[1] + 0.5*all_bbox[j][1];
                    float y11 = 0.5*tmp_box[2] + 0.5*all_bbox[j][2];
                    float x22 = 0.5*tmp_box[3] + 0.5*all_bbox[j][3];
                    float y22 = 0.5*tmp_box[4] + 0.5*all_bbox[j][4];
                    tmp_.push_back(t_idx);
                    tmp_.push_back(x11);
                    tmp_.push_back(y11);
                    tmp_.push_back(x22);
                    tmp_.push_back(y22);

                    all_bbox.erase(all_bbox.begin() + j);
                    // all_bbox.push_back(tmp_box);
                    all_bbox.push_back(tmp_);
                    tmp_.clear();
                    */
                }
                p = 1;
            }
        }
        if (p == 0)
        {
            all_bbox.push_back(tmp_box);
        }
        tmp_box.clear();
    }

    for (int i = 0; i < this->output_stracks.size(); i++)
    {
        for (int j = 0; j < all_bbox.size(); j++)
        {
            if (this->output_stracks[i].track_id == all_bbox[j][0])
            {
                this->output_stracks[i].tlwh[0] = all_bbox[j][1];
                this->output_stracks[i].tlwh[1] = all_bbox[j][2];
                this->output_stracks[i].tlwh[2] = all_bbox[j][3] - all_bbox[j][1];
                this->output_stracks[i].tlwh[3] = all_bbox[j][4] - all_bbox[j][2];
                this->output_stracks[i].cls_id = all_bbox[j][5];
            }
        }

    }

    //**************************************************************************

    //auto track_end = std::chrono::system_clock::now();
    //auto track_time =std::chrono::duration_cast<std::chrono::milliseconds>(track_end - track_start).count();//ms
    //std::cout << "track time : " << track_time << "ms" << std::endl;

}