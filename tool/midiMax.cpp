#include "Options.h"
#include "midiLoader.h"
#include "midiMap.h"
#include "sampler.h"
int main(int argc, char** argv) {
    bool processError = false;
    if (argc < 3) {
        return 0;
    }
    std::string input = argv[1];
    std::string output_ori = std::string(argv[2]);
    std::cout << "输入:" << input << std::endl;

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap, input);
    midiMap.updateTimeMax();

    int lenInBeat = (midiMap.noteTimeMax / midiMap.TPQ);
    std::vector<int> notes;
    char channel_note[64];
    int numNote = 0;
    for (int i = 0; i < lenInBeat; i = i + 1) {
        for (int j = 0; j < 4; ++j) {
            notes.push_back(mgnr::sample(midiMap, i + j * 0.25, 0.25, 0, [&](mgnr::note* n) {
                return true;
            }));
            ++numNote;
        }
    }
    //std::cout << "旋律:" << numNote << "和弦:" << numChord << std::endl;
    if(processError){
        std::cout << "出现错误，停止写入" << std::endl;
        return 1;
    }
    std::cout << "旋律:" << notes.size()<< std::endl;
    auto fp = fopen(output_ori.c_str(), "a");
    if (fp) {
        if (argc >= 6) {
            fprintf(fp, "%s %s|[", input.c_str(),argv[5]);
        }else{
            fprintf(fp, "%s|[", input.c_str());
        }
        bool first = true;
        for (auto it : notes) {
            if (first) {
                fprintf(fp, "%d", it);
            } else {
                fprintf(fp, ",%d", it);
            }
            first = false;
        }
        fprintf(fp, "]\n");
        fclose(fp);
    }
    return 0;
}
