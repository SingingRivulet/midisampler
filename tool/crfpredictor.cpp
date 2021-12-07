#include <crfpp.h>
#include <iostream>
int main() {
    CRFPP::Tagger* tagger = CRFPP::createTagger("-m model -v 3 -n2");

    if (!tagger) {
        std::cerr << CRFPP::getTaggerError() << std::endl;
        return 1;
    }
    // clear internal context
    tagger->clear();

    // add context
    tagger->add("6 dim3 I");
    tagger->add("3 dim3 I");
    tagger->add("3 single II");
    tagger->add("3 dim3 II");
    tagger->add("3 dim3 III");
    tagger->add("5 single IV");
    tagger->add("6 single IV");
    tagger->add("3 single V");
    tagger->add("3 single V");
    tagger->add("1 single VI");
    tagger->add("1 single VI");
    tagger->add("5 single VII");
    tagger->add("5 single VII");

    std::cout << "column size: " << tagger->xsize() << std::endl;
    std::cout << "token size: " << tagger->size() << std::endl;
    std::cout << "tag size: " << tagger->ysize() << std::endl;

    // parse and change internal stated as 'parsed'
    if (!tagger->parse())
        return -1;

    std::cout << "conditional prob=" << tagger->prob()
              << " log(Z)=" << tagger->Z() << std::endl;

    for (size_t i = 0; i < tagger->size(); ++i) {
        for (size_t j = 0; j < tagger->xsize(); ++j) {
            std::cout << tagger->x(i, j) << '\t';
        }
        std::cout << "predict:";
        std::cout << tagger->y2(i) << '\t';
        std::cout << std::endl;
    }

    std::cout << "Done" << std::endl;

    delete tagger;

    return 0;
}