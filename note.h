#ifndef MGNR_MIDI_NOTE
#define MGNR_MIDI_NOTE
#include <string>
namespace mgnr{
    struct noteIndex{
        double   start;
        int     id;
        inline noteIndex(double s,int i){
            start   =s;
            id      =i;
        }
        inline noteIndex(){
            start   =0;
            id      =0;
        }
        inline noteIndex(const noteIndex & i){
            start   =i.start;
            id      =i.id;
        }
        inline const noteIndex & operator=(const noteIndex & i){
            start   =i.start;
            id      =i.id;
            return *this;
        }
        inline bool operator<(const noteIndex & i)const{
            return (start<i.start && id<i.id);
        }
        inline bool operator==(const noteIndex & i)const{
            return (start==i.start && id==i.id);
        }
    };
    class note{
        public:
            double begin;
            double tone;
            double duration;
            int volume;
            int id;
            int startId;
            int endId;
            int ins_id;
            
            std::string info;
            
            bool selected;
            bool playing;
            long playTimes;
            
            inline void construct(){
                selected  = false;
                playing   = false;
                playTimes =0;
            }
            
            noteIndex beginIndex,endIndex;
            
            inline void getBeginIndex(){
                beginIndex = noteIndex(begin,startId);
            }
            inline void getEndIndex(){
                endIndex = noteIndex(begin+duration,endId);
            }
            
            void * indexer;
            
            note * next;//内存池变量，不应该被修改
    };
}
#endif
