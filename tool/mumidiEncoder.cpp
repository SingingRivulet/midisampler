#include <math.h>
#include "Options.h"
#include "midiLoader.h"
#include "midiMap.h"
#include "sampler.h"
#include <sstream>
#include <cassert>

template <typename T>
void get_beat_combinations(T callback){
    for (int beat_offset=0;beat_offset<4;++beat_offset){
        for (int time_sig=3;time_sig<=4;++time_sig){
            callback(beat_offset,time_sig);
        }
    }
}

std::map<mgnr::noteIndex,mgnr::note*>::iterator findClosest(std::map<mgnr::noteIndex,mgnr::note*>& myMap, mgnr::noteIndex key) {
    if (myMap.empty())
        return myMap.end();

    auto it = myMap.lower_bound(key); // Find the first element that is not less than key

    // If it is the first element or the key is less than or equal to the first element
    if (it == myMap.begin() || (it != myMap.end() && std::abs(it->first.start - key.start) < std::abs(std::prev(it)->first.start - key.start))) {
        return it;
    } else {
        return std::prev(it);
    }
}

float get_inbeat_dist(double time_sig, double time_offset, mgnr::midiMap& midiMap) {
    double time_delta_sum = 0;
    int event_num = 0;
    for (double sample_pos = time_offset*midiMap.TPQ; sample_pos < midiMap.noteTimeMax; sample_pos += time_sig*midiMap.TPQ){
        auto cit = findClosest(midiMap.timeIndex, mgnr::noteIndex{sample_pos, 0});
        if (cit != midiMap.timeIndex.end()) {
            time_delta_sum += abs(sample_pos - cit->first.start);
            ++event_num;
        }
    }
    if (event_num == 0) {
        return 0;
    }
    return time_delta_sum / event_num;
}

std::string get_token_note(double delta_beat, double duration_beat, int pitch) {
    if (pitch<=0 || pitch>=128) {
        return "";
    }
    int delta_beat_q = std::min(int(delta_beat*36), 36*4);
    int duration_beat_q = std::min(int(duration_beat*36), 36*8);
    int delta_token = delta_beat_q + 128;
    int duration_token = duration_beat_q + 128 + 36*4;
    if (delta_beat_q<0 || duration_beat_q<0) {
        return "";
    }
    std::ostringstream oss;
    oss << pitch << " " << delta_token << " " << duration_token;
    return oss.str();
}
std::string get_token_bar(int time_sig, double current_tempo) {
    assert(time_sig>=1 && time_sig<=4);
    int tempo_q = current_tempo/10;
    if (tempo_q < 1){
        tempo_q = 1;
    }
    if (tempo_q > 60){
        tempo_q = 60;
    }
    int time_sig_token = time_sig + 128 + 36*4 + 36*8;
    int tempo_token = tempo_q + 128 + 36*4 + 36*8 + 5;
    std::ostringstream oss;
    oss << time_sig_token << " " << tempo_token;
    return oss.str();
}
std::string get_token_instrument(int ins_id) {
    assert(ins_id>=0 && ins_id<=128);
    int ins_token = ins_id + 128 + 36*4 + 36*8 + 5 + 60;
    std::ostringstream oss;
    oss << ins_token;
    return oss.str();
}

int main(int argc, const char** argv) {
    bool processError = false;
    if (argc < 5) {
        std::cout << "usage:" << argv[0] << " time_sig time_offset input_file output_file" << std::endl;
        return 0;
    }
    // 拍号
    int time_sig = atoi(argv[1]);
    // 偏移
    double time_offset = atof(argv[2]);
    // 输入文件
    std::string input = argv[3];
    // 输出文件
    std::string output = std::string(argv[4]);

    std::cout << "输入:" << input << std::endl;

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap, input);
    midiMap.updateTimeMax();
    // 计算长度
    double lenInBeat = (midiMap.noteTimeMax / midiMap.TPQ);
    std::cout << "长度：" << lenInBeat << std::endl;

    // 如果time_sig为负，计算节拍
    if (time_sig<=0) {
        std::cout << "尝试估计节拍..." << std::endl;
        // 穷举1、2、3、4，找到碰到缝隙概率最大的
        double min_inbeat_dist = INFINITY;
        double min_time_sig = 4;
        double min_time_offset = 0;
        get_beat_combinations([&](int beat_offset, int time_sig){
            double inbeat_dist = get_inbeat_dist(time_sig, beat_offset, midiMap);
            if (inbeat_dist < min_inbeat_dist) {
                min_inbeat_dist = inbeat_dist;
                min_time_sig = time_sig;
                min_time_offset = beat_offset;
            }
        });
        time_sig = min_time_sig;
        time_offset = min_time_offset + time_offset;
    }

    int num_sample = ceil((lenInBeat - time_offset) / time_sig);
    std::cout << "节拍：" << time_sig << " (共" << num_sample << "小节)" << std::endl;
    std::cout << "偏移：" << time_offset << std::endl;

    int token_seg_size = 2048;
    std::string token;
    std::string token_seg_buffer;
    int token_seg_buffer_count = 0;
    std::vector<std::tuple<std::string,int>> token_segs;

    // 创建输出文件
    auto fp = fopen(output.c_str(), "w");
    if (fp) {
        fprintf(fp, "<mumidi>\n  <events max=%d>\n", 128 + 36*4 + 36*8 + 5 + 60 + 128);
        for (int index = 0; index < num_sample; ++index) {
            std::map<int, std::vector<mgnr::note*>> notes_bar{};
            double sample_beat = time_sig * index + time_offset;
            long sample_tick = midiMap.TPQ * sample_beat;
            long sample_tick_end = midiMap.TPQ * (sample_beat + time_sig);
            mgnr::fetch(midiMap, sample_beat, time_sig, [&](auto n) {
                if (n->begin >= sample_tick && n->begin < sample_tick_end) {
                    // double current_beat = n->begin/midiMap.TPQ;
                    auto ins_id = n->ins_id;
                    notes_bar[ins_id].push_back(n);
                }
            });
            
            std::string token_seg_bar{};
            int token_seg_bar_num = 0;

            double current_tempo = midiMap.getTempo(sample_tick);
            auto ev_token = get_token_bar(time_sig, current_tempo);
            token += ev_token + " ";
            fprintf(fp, "    <bar time_sig=%d tempo=%lg token=\"%s\">\n", time_sig, current_tempo, ev_token.c_str());
            
            token_seg_bar_num += 2;
            token_seg_bar += ev_token + " ";
            
            for (auto& notes_group : notes_bar) {
                ev_token = get_token_instrument(notes_group.first);
                token += ev_token + " ";
                fprintf(fp, "      <instrument id=%d token=\"%s\">\n", notes_group.first, ev_token.c_str());

                token_seg_bar_num += 1;
                token_seg_bar += ev_token + " ";

                auto& notes = notes_group.second;
                std::sort(notes.begin(), notes.end(), [](auto& a, auto& b) {
                    return a->begin < b->begin;
                });
                for (auto& note : notes) {
                    double current_beat = note->begin / midiMap.TPQ;
                    double delta_beat = current_beat - sample_beat;
                    double duration_beat = note->duration / midiMap.TPQ;
                    ev_token = get_token_note(delta_beat, duration_beat, note->tone);
                    token += ev_token + " ";
                    fprintf(fp, "        <note time=%lg duration=%lg pitch=%lg token=\"%s\"/>\n", delta_beat, duration_beat, note->tone, ev_token.c_str());

                    token_seg_bar_num += 3;
                    token_seg_bar += ev_token + " ";
                }
                fprintf(fp, "      </instrument>\n");
            }
            fprintf(fp, "    </bar>\n");
            if (token_seg_bar_num + token_seg_buffer_count >= token_seg_size){
                token_segs.push_back(std::make_tuple(token_seg_buffer, token_seg_buffer_count));
                token_seg_buffer_count = token_seg_bar_num;
                token_seg_buffer = token_seg_bar;
            }else{
                token_seg_buffer_count += token_seg_bar_num;
                token_seg_buffer += token_seg_bar;
            }
        }
        if (token_seg_buffer_count > 0){
            token_segs.push_back(std::make_tuple(token_seg_buffer, token_seg_buffer_count));
        }
        fprintf(fp, "  </events>\n  <tokens>\n    %s\n  </tokens>\n  <segs>\n", token.c_str());
        for (auto & it:token_segs){
            fprintf(fp, "    <seg size=%d>\n      %s\n    </seg>\n", std::get<1>(it), std::get<0>(it).c_str());
        }
        fprintf(fp, "  </segs>\n</mumidi>\n");
        fclose(fp);
    }
    return 0;
}
