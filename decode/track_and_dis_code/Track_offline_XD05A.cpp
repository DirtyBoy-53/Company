// Track_offline_XD05A.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <numeric>
#include "common.h"
#include <vector>
#include "BYTETracker.h"
#include <string>
#include <filesystem>
#include <regex>
#include "Mono_dis.h"
#include "Define_struct.h"

using namespace std;
namespace fs = std::filesystem;

static BYTETracker* tracker = nullptr;

vector<string> s_split(const std::string& in, const std::string& delim) {
    regex re{ delim };
    return vector<string> {
        sregex_token_iterator(in.begin(), in.end(), re, -1),
            sregex_token_iterator()
    };
}

void read_labels(const string label_path, vector<Object>& bboxes) {
    
    ifstream ifs;
    ifs.open(label_path, ios::in);
    if (ifs.is_open()) {
        string line;
        while (getline(ifs, line)) {
            vector<string> data = s_split(line, " ");
            Object box;
            box.label = atoi(data[0].c_str());
            box.prob = atof(data[5].c_str());
            box.rect.x = atof(data[1].c_str()) * 640.f;
            box.rect.y = atof(data[2].c_str()) * 512.f;
            box.rect.width = atof(data[3].c_str()) * 640.f;
            box.rect.height = atof(data[4].c_str()) * 512.f;
            box.rect.x = box.rect.x - box.rect.width / 2.f;
            box.rect.y = box.rect.y - box.rect.height / 2.f;
            bboxes.push_back(box);
        }
    }
}

// for case4
void read_labels_2(const string label_path, vector<Object>& bboxes) {

    ifstream ifs;
    ifs.open(label_path, ios::in);
    if (ifs.is_open()) {
        string line;
        while (getline(ifs, line)) {
            vector<string> data = s_split(line, ",");
            Object box;
            box.label = atoi(data[0].c_str());
            box.prob = atof(data[5].c_str());
            box.rect.x = atof(data[1].c_str());
            box.rect.y = atof(data[2].c_str());
            box.rect.width = atof(data[3].c_str());
            box.rect.height = atof(data[4].c_str());
            box.rect.width = box.rect.width - box.rect.x;
            box.rect.height = box.rect.height - box.rect.y;
            bboxes.push_back(box);
        }
    }
}

int main()
{
    string data_root = "data/case1/image";
    string str1 = "image";
    string str2 = "predict/labels";
    string str3 = "save/labels";
    string str4 = "txt";
    tracker = new BYTETracker(30, 30);

    external_param_t external_info;
    external_info.h = 0;
    external_info.m_install_height = 0.666;
    external_info.m_pitch = 0.9 / 180 * 3.1415926;
    external_info.m_yaw = 0.45 / 180 * 3.1415926;
    external_info.m_roll = 0.26 / 180 * 3.1415926;
    float m_intrinsic_param1[] = { 748.487, 0, 323.83, 0, 748.487, 258.237, 0, 0, 1, -0.342252, 0.0905441, 0, 0, 0, 0.9, 3};

    img_detect_info_m img_detect_info;
    img_detect_info.car_PDCU_ShiftLvlPosn = 0;
    img_detect_info.car_speed = 60;
    img_detect_info.flag_dist_switch = false;
    img_detect_info.frame_id = 10;
    img_detect_info.imu_pitch = 0;
    img_detect_info.imu_roll = 0;
    img_detect_info.imu_yaw = 0;

    std::vector<detectbox_info_m> detectbox_info;

    int count = 0;
    for (auto& entry : fs::directory_iterator(data_root)) {
        if (fs::is_regular_file(entry)) {
            string image_path = entry.path().string();
            string label_path = entry.path().string().replace(22, str4.size(), str4).replace(11, str1.size(), str2);
            string save_path = entry.path().string().replace(22, str4.size(), str4).replace(11, str1.size(), str3);

            vector<Object> bboxes;
            read_labels(label_path, bboxes);
            cout << label_path << endl;
            tracker->update(bboxes);
            if (count == 1400) {
                cout << "xxx" << endl;
            }

            ofstream out(save_path);
            for (int j = 0; j < tracker->output_stracks.size(); j++) {
                out << tracker->output_stracks[j].track_id << " "
                    << tracker->output_stracks[j].tlwh[0] << " "
                    << tracker->output_stracks[j].tlwh[1] << " "
                    << tracker->output_stracks[j].tlwh[2] + tracker->output_stracks[j].tlwh[0] << " "
                    << tracker->output_stracks[j].tlwh[3] + tracker->output_stracks[j].tlwh[1] << " "
                    << tracker->output_stracks[j].score << endl;
            }
            out.close();

            img_detect_info.frame_id = count;
            count++;
            detectbox_info.clear();
            detectbox_info.resize(tracker->output_stracks.size());
            for (int j = 0; j < tracker->output_stracks.size(); j++) {
                detectbox_info[j].track_id = tracker->output_stracks[j].track_id;
                detectbox_info[j].obj_id = tracker->output_stracks[j].cls_id;
                detectbox_info[j].detectbox_x_left = tracker->output_stracks[j].tlbr[0];
                detectbox_info[j].detectbox_x_right = tracker->output_stracks[j].tlbr[2];
                detectbox_info[j].detectbox_y_top = tracker->output_stracks[j].tlbr[1];
                detectbox_info[j].detectbox_y_bottom = tracker->output_stracks[j].tlbr[3];
                detectbox_info[j].ai_dist = 0;
            }
            img_detect_info.detectbox_info = detectbox_info;
            int returnValueErr = IRCalib::Instance()->calc_distance(img_detect_info, m_intrinsic_param1, &external_info, 0.5, 60);

        }
    }
}

