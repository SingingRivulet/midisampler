#include <math.h>
#include "Options.h"
#include "midiLoader.h"
#include "midiMap.h"
#include "sampler.h"
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
    int num_sample = ceil((lenInBeat - time_offset) / time_sig);
    std::cout << "长度：" << lenInBeat << std::endl;

    // 创建输出文件
    auto fp = fopen(output.c_str(), "a");
    if (fp) {
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
            fprintf(fp, "bar:time_sig=%d tempo=%lg\n", time_sig, midiMap.getTempo(sample_tick));
            for (auto& notes_group : notes_bar) {
                fprintf(fp, "instrument:id=%d\n", notes_group.first);
                auto& notes = notes_group.second;
                std::sort(notes.begin(), notes.end(), [](auto& a, auto& b) {
                    return a->begin < b->begin;
                });
                for (auto& note : notes) {
                    double current_beat = note->begin / midiMap.TPQ;
                    double delta_beat = current_beat - sample_beat;
                    double duration_beat = note->duration / midiMap.TPQ;
                    fprintf(fp, "note:time=%lg duration=%lg pitch=%lg\n", delta_beat, duration_beat, note->tone);
                }
            }
        }
        fclose(fp);
    }
    return 0;
}
