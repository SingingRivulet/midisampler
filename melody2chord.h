#ifndef MGNR_MELODY2CHORD
#define MGNR_MELODY2CHORD
#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <map>
namespace mgnr{
    
    inline void getComp(//求组合
        const std::vector<std::tuple<int,float> > & notes, //输入已排序的集合{(音符，音符的数量),...}
        const std::function<void(const std::vector<std::tuple<int,float> >&,int weight)> & callback //回调函数(音符集合,需要改变的数量)
    ){
        //求音符总数
        float sum = 0;
        for(auto it:notes){
            sum += std::get<1>(it);
        }
        //开始求组合
        int len = notes.size();
        std::vector<std::tuple<int,float> > res;
        for(int i=0;i<len;++i){
            res.clear();
            float compCount = 0;
            for(int j=i;j<len;++j){
                auto it = notes.at(j);
                res.push_back(it);
                compCount += std::get<1>(it);
            }
            callback(res,sum-compCount);
        }
    }
    float calcEditDist(const std::vector<std::tuple<int,float> >& A, const std::vector<int>& B) {
        int lenA = A.size();
        int lenB = B.size();
        int i, j;
        std::vector<std::vector<float>> dp(lenA + 1, std::vector<float>(lenB + 1));

        //初始化边界
        for (i = 1; i <= lenA; i++) {
            dp[i][0] = std::get<1>(A[i-1])+dp[i-1][0];
        }
        for (j = 1; j <= lenB; j++) {
            dp[0][j] = j;
        }

        //dp
        for (i = 1; i <= lenA; i++) {
            for (j = 1; j <= lenB; j++) {
                //if (s1[i] == s2[j])//运行结果不对，因为第i个字符的索引为i-1
                if (std::get<0>(A[i - 1]) == B[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];  //第i行j列的步骤数等于第i-1行j-1列，因为字符相同不需什么操作，所以不用+1
                } else {
                    dp[i][j] = std::min(std::min(
                        dp[i - 1][j] + std::get<1>(A[i - 1]),
                        dp[i][j - 1] + std::get<1>(A[i])) ,
                        dp[i - 1][j - 1] + std::get<1>(A[i - 1]))+1;  //+1表示经过了一次操作
                }
            }
        }
        #ifdef MGNR_DEBUG
        printf("lenA=%d lenB=%d\n",lenA,lenB);
        for (i = 0; i <= lenA; i++) {
            for (j = 0; j <= lenB; j++) {
                printf("%f\t",dp[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        #endif

        return dp[lenA][lenB];
    }

    inline void notes2RelativeChord(
        const std::vector<std::tuple<int,float> > & notes,
        std::vector<std::tuple<int,float>> & relativeChord
    ){
        bool first = true;
        int firstNote = std::get<0>(notes.at(0));
        relativeChord.clear();
        for(auto it:notes){
            relativeChord.push_back(std::make_tuple(std::get<0>(it)-firstNote , std::get<1>(it)));
        }
    }

    struct chordMap{
        std::map<std::string,std::vector<int> > chord_map;
        chordMap(){
            chord_map["single"] =  {0};   // 单音
            chord_map["maj3"] =  {0, 4, 7};   // 大三和弦 根音-大三度-纯五度
            chord_map["min3"] =  {0, 3, 7};   //小三和弦 根音-小三度-纯五度
            chord_map["aug3"] =  {0, 4, 8};   //增三和弦 根音-大三度-增五度
            chord_map["dim3"] =  {0, 3, 6};   //减三和弦 根音-小三度-减五度
            chord_map["M7"]   =  {0, 4, 7, 11};  //大七和弦 根音-大三度-纯五度-大七度
            chord_map["Mm7"]  =  {0, 4, 7, 10};  //属七和弦 根音-大三度-纯五度-小七度
            chord_map["m7"]   =  {0, 3, 7, 10};  //小七和弦 根音-小三度-纯五度-小七度
            chord_map["mM7"]  =  {0, 3, 7, 11};  //小大七和弦 根音-小三度-纯五度-大七度
            chord_map["aug7"] =  {0, 4, 8, 10};  //增七和弦 根音-大三度-增五度-小七度
            chord_map["augM7"]=  {0, 4, 8, 11};  //增大七和弦 根音-大三度-增五度-小七度
            chord_map["m7b5"] =  {0, 3, 6, 10};  //半减七和弦 根音-小三度-减五度-减七度
            chord_map["dim7"] =  {0, 3, 6, 9};   //减减七和弦 根音-小三度-减五度-减七度
        }
    };

    template<typename T>
    inline std::tuple<int,std::string> noteSet2Chord(T & self,const std::vector<std::tuple<int,float> > & notes){
        if(notes.empty()){
            return std::make_tuple(0,"_");
        }
        std::string min_chord;
        int min_base;
        float min_chord_weight;
        bool first = true;
        getComp(notes,[&](const std::vector<std::tuple<int,float> > & n,int baseWeight){
            std::vector<std::tuple<int,float> > relative;
            notes2RelativeChord(n,relative);
            #ifdef MGNR_DEBUG
            printf("note:");
            for(auto it:relative){
                printf("%d:%f ",std::get<0>(it),std::get<1>(it));
            }
            printf("\n");
            #endif
            for(auto & it:self.chord_map){
                #ifdef MGNR_DEBUG
                printf("chord_map:%s ",it.first.c_str());
                for(auto iit:it.second){
                    printf("%d ",iit);
                }
                #endif
                float weight = calcEditDist(relative,it.second)+baseWeight;
                #ifdef MGNR_DEBUG
                printf("=>%f(%d)\n",weight,baseWeight);
                #endif
                if(first || weight<min_chord_weight){
                    min_chord_weight = weight;
                    min_chord = it.first;
                    min_base = std::get<0>(n.at(0));
                }
                first = false;
            }
        });
        return std::make_tuple(min_base,min_chord);
    }
    template<typename T>
    inline std::tuple<int,std::string,std::vector<int> > noteSeq2Chord(T & self,const std::vector<int> & seq){
        std::map<int,int> count;
        for(auto it:seq){
            if(it>0){
                ++count[it];
            }
        }
        std::vector<std::tuple<int,float> > noteset;
        for(auto it:count){
            noteset.push_back(std::make_tuple(it.first,(float)it.second));
        }
        auto chord = noteSet2Chord(self,noteset);
        std::vector<int> notes;
        return std::make_tuple(std::get<0>(chord),std::get<1>(chord),notes);
    }
}
#endif