#pragma once

#include "STrack.h"
#include "common.h"

class BYTETracker
{
public:
    BYTETracker(int frame_rate = 30, int track_buffer = 30);
    ~BYTETracker();

    void update(const vector<Object>& objects);
    Scalar get_color(int idx);


    vector<STrack> output_stracks;

private:
    vector<STrack*> joint_stracks(vector<STrack*>& tlista, vector<STrack>& tlistb);
    vector<STrack> joint_stracks(vector<STrack>& tlista, vector<STrack>& tlistb);

    vector<STrack> sub_stracks(vector<STrack>& tlista, vector<STrack>& tlistb);
    void remove_duplicate_stracks(vector<STrack>& resa, vector<STrack>& resb, vector<STrack>& stracksa, vector<STrack>& stracksb);

    void linear_assignment(vector<vector<float> >& cost_matrix, int cost_matrix_size, int cost_matrix_size_size, float thresh,
        vector<vector<int> >& matches, vector<int>& unmatched_a, vector<int>& unmatched_b);
    vector<vector<float> > iou_distance(vector<STrack*>& atracks, vector<STrack>& btracks, int& dist_size, int& dist_size_size);
    vector<vector<float> > iou_distance(vector<STrack>& atracks, vector<STrack>& btracks);
    //vector<vector<float> > ious(vector<vector<float> > &atlbrs, vector<vector<float> > &btlbrs, vector<int> &acls, vector<int> &bcls);
    vector<vector<float> > ious(vector<vector<float> >& atlbrs, vector<vector<float> >& btlbrs);

    double lapjv(const vector<vector<float> >& cost, vector<int>& rowsol, vector<int>& colsol,
        bool extend_cost = false, float cost_limit = LONG_MAX, bool return_cost = true);

    vector<vector<float> > fuse_score(vector<vector<float>>& cost_matrix, vector<STrack>& detections);

private:

    int img_w_;
    int img_h_;

    float track_thresh;
    float high_thresh;
    float match_thresh;
    int frame_id;
    int max_time_lost;

    float new_stracks_thresh_ped;
    float max_ped_area;
    int max_supplementary_box_count;

    vector<STrack> tracked_stracks;
    vector<STrack> lost_stracks;
    vector<STrack> removed_stracks;
    byte_kalman::KalmanFilter kalman_filter;


};