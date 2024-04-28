#include "../midiMap.h"
#include "../midiLoader.h"
#include "../sampler.h"
#include <stdlib.h>
int main(int argc, char** argv) {
    if (argc < 5) {
        printf("Usage: %s <input> <output> <melody_track_id> <min_beat>\n", argv[0]);
        printf("example: %s input.mid output.mid 3 0.125\n", argv[0]);
        return 0;
    }
    std::string input = argv[1];
    std::string output = argv[2];
    int melody_track_id = atoi(argv[3]);
    float min_beat = atof(argv[4]);
    smf::MidiFile midifile;
    midifile.read(input);
    // remove short notes
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    int tracks = midifile.getTrackCount();
    auto tpq = midifile.getTicksPerQuarterNote();
    std::cout << "TPQ: " << tpq << std::endl;
    std::cout << "num tracks " << tracks << std::endl;
    if ( tracks <= melody_track_id ){
        std::cout << "track " << melody_track_id << " not found" << std::endl;
        return 1;
    }
    auto& melody_track = midifile[melody_track_id];
    std::vector<std::tuple<int,int,int,int,int>> notes;
    for (int event_id=0; event_id<melody_track.size(); event_id++) {
        auto & event = melody_track[event_id];
        if (event.isNoteOn()){
            int position = event.tick;
            int duration = event.getTickDuration();
            int pitch = (int)event[1];
            int v = (int)event[2];
            int channel = event.getChannel();
            float duration_beat = (float)duration / tpq;
            std::cout << position << "\t" << 
                duration << "\t" << 
                pitch << "\t" << v << "\t" << duration_beat << std::endl;
            if (duration_beat >= min_beat){
                notes.push_back(std::make_tuple(position, duration, pitch, v, channel));
            }
        }
    }
    //删除前一个音符延续到后一个音符的部分
    for (int i=0; i<notes.size()-1; i++){
        auto &n1 = notes[i];
        auto &n2 = notes[i+1];
        auto &dur1 = std::get<1>(n1);
        auto &dur2 = std::get<1>(n2);
        auto &pos1 = std::get<0>(n1);
        auto &pos2 = std::get<0>(n2);
        if (pos1+dur1>pos2){
            dur1 = pos2-pos1;
        }
    }

    melody_track.clear();
    for (auto &note: notes){
        auto &pos = std::get<0>(note);
        auto &dur = std::get<1>(note);
        auto &pitch = std::get<2>(note);
        auto &v = std::get<3>(note);
        auto &channel = std::get<4>(note);
        midifile.addNoteOn (melody_track_id, pos, channel, pitch, v);
        midifile.addNoteOff(melody_track_id, pos+dur,   channel, pitch);
    }
    midifile.sortTracks();
    midifile.write(output);

    return 0;
}