#include <algorithm>
#include <iterator>
#include <list>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class InfoBlock {
    public:
        enum InfoBlockType {
            kInfoBlockData,
            kInfoBlockEnter,
            kInfoBlockExit,
        };

        InfoBlock(InfoBlockType type, std::string data = "") 
            : type_(type)
            , data_(data)
            , pos_(nullptr)
            , dataHashVal_(0)
            , repeatCount_(0)
            , mapKey_("")
            {
                updateDataHashVal();
            }

        ~InfoBlock() {

        }

        void appendData(std::string data) {
            data_ = data_ + data;
            updateDataHashVal();
        }

        const std::string &data() {
            return data_;
        }

        InfoBlockType type() {
            return type_;
        }

        bool operator==(const InfoBlock &infoBlock) {
            return type_ == infoBlock.type_ 
                // && dataHashVal_ == infoBlock.dataHashVal_
                && data_ == infoBlock.data_;
        }

        bool operator!=(const InfoBlock &infoBlock) {
            return !(*this == infoBlock);
        }

        void setPos(std::list<InfoBlock *>::iterator pos) {
            pos_ = std::move(pos);
        }

        void setMPos(std::unordered_multimap<std::string, InfoBlock *>::iterator mpos) {
            mpos_ = std::move(mpos);
        }

        const std::unordered_multimap<std::string, InfoBlock *>::iterator & mPos() const {
            return mpos_;
        }

        const std::list<InfoBlock *>::iterator & pos() const {
            return pos_;
        }

        uint32_t dataHashVal() const {
            return dataHashVal_;
        }

        uint32_t repeatCount() const {
            return repeatCount_;
        }

        void addRepeatCount(int n = 1) {
            repeatCount_ += n;
        }

        void setMapKey(std::string mapKey) {
            mapKey_ = std::move(mapKey);
        }

        const std::string &mapKey() const {
            return mapKey_;
        }

        uint32_t line_;

    private:
        void updateDataHashVal() {
            uint32_t v = 0;
            for (char c  : data_) {
                v += c;
            }
            dataHashVal_ = v;
        }

        InfoBlockType type_;
        std::string data_;
        std::list<InfoBlock*>::iterator pos_;
        std::unordered_multimap<std::string, InfoBlock *>::iterator mpos_;
        uint32_t dataHashVal_;
        uint32_t repeatCount_;
        std::string mapKey_;
};

void usage(const char *prgname)
{
    fprintf(stderr, "Usage: %s [-i inputFile] [-o outFile]\n",
            prgname);
}

int main (int argc, char *argv[]) {

    int opt;
    std::string inFileName, outFileName;

    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
            case 'i':
                inFileName = optarg;
                break;
            case 'o':
                outFileName = optarg;
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (inFileName.empty() || outFileName.empty()) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    std::cout << "inputFile [" << inFileName << "] outputFile [" << outFileName << "]" << std::endl;

    std::ifstream inFile(inFileName);

    if (!inFile.is_open()) {
        std::cerr << "Error : Cant open " << inFileName << ": " << strerror(errno) << std::endl;
        return -1;
    }

    bool isEnd = false;
    std::string line;
    uint32_t lineNb = 0;

    std::string sEnter("Enter"), sExit("Exit");

    InfoBlock *b = nullptr;
    InfoBlock::InfoBlockType type = InfoBlock::kInfoBlockData;

    std::list<InfoBlock *> infoBlockList;
    std::unordered_multimap<std::string, InfoBlock *> infoBlockMap;

    int minEqualNb = 1;

    // load file
    while(!isEnd) {
        std::getline(inFile, line);

        if (inFile.eof()) {
            isEnd = true;
        }
        else {
            lineNb++;
        }

        type = InfoBlock::kInfoBlockData;
        if (line == sEnter) {
            // std::cout << "find enter" << std::endl;
            type = InfoBlock::kInfoBlockEnter;
        }
        else if (line == sExit) {
            // std::cout << "find exit" << std::endl;
            type = InfoBlock::kInfoBlockExit;
        }

        line = line + "\n";

        if (type != InfoBlock::kInfoBlockData) {

            if (b) {
                infoBlockList.emplace_back(b);
                auto it = infoBlockList.end();
                --it;
                b->setPos(it);
                b->line_ = lineNb;
            }

            b = new InfoBlock(type, line);
        }
        else {
            if (!b) {
                std::cout << "WARN: 忽略无头信息的行[" << lineNb << "]" << std::endl; 
                continue;
            }
            else {
                b->appendData(line);
            }
        }

    }

    std::cout << "load file : ok" << std::endl;

    // map
    for (auto itList = infoBlockList.begin(); itList != infoBlockList.end(); ++itList) {
        int n = std::distance(itList, infoBlockList.end());
        if (n < minEqualNb) {
            std::cout << __LINE__ << ": " << n << " < " << minEqualNb << std::endl;
            continue;
        }

        std::string key;
        auto itPos = itList;
        for (int i = 0; i < minEqualNb; ++i, ++itPos) {
            key += (*itPos)->data();
        }

        auto itMap = infoBlockMap.insert(std::make_pair(key, *itList));
        (*itList)->setMapKey(key);
        (*itList)->setMPos(itMap);
    }

    std::cout << "create map : ok" << std::endl;

    std::list<InfoBlock *>::iterator hintBegin = infoBlockList.end();
    while (1) {
__next__:
        // std::for_each(infoBlockMap.begin(), infoBlockMap.end(), 
        //         [&](std::pair<std::string, InfoBlock *> pair) {
        //         std::cout << "line : " << pair.second->line_ << std::endl;
        //         // std::cout << "distance :" << std::distance(infoBlockList.begin(), pair.second->pos()) << std::endl;
        //         });


        // std::cout << "infoBlockList.len : " << infoBlockList.size() << std::endl;
        // std::cout << "infoBlockMap.len : " << infoBlockMap.size() << std::endl;
        // getchar();

        std::list<InfoBlock *>::iterator itList;
        if (hintBegin != infoBlockList.end()) {
            itList = hintBegin;
            hintBegin = infoBlockList.end();
        }
        else {
            itList = infoBlockList.begin();
        }
            // itList = infoBlockList.begin();

        for (; itList != infoBlockList.end(); ++itList) {

            if ((*itList)->mapKey().empty()) {
                continue;
            }

            auto range = infoBlockMap.equal_range((*itList)->mapKey());

            int seq = std::distance(infoBlockList.begin(), itList);

            int rangeLen = std::distance(range.first, range.second) + 1;

            if (rangeLen == 1)
                continue;

            std::cout << "infoBlockList size: " << infoBlockList.size() << std::endl;
            std::cout << "infoBlockMap size: " << infoBlockMap.size() << std::endl;
            std::cout << "seq: " << seq << ", rangeLen: " << rangeLen << std::endl;

            for (auto itMap = range.first; itMap != range.second; ) {
                if ((*itList) == itMap->second) {
                    ++itMap;
                    // itMap = infoBlockMap.erase(itMap);
                    continue;
                }

                std::cout << "target key : " << (*itList)->mapKey() << std::endl;
                std::cout << std::distance(infoBlockList.begin(), itList) << " cmp "<< std::distance(infoBlockList.begin(), itMap->second->pos()) << std::endl;

                // std::cout << "curMap key : " << itMap->second->mapKey() << std::endl;
                // if ((*itList)->mapKey() == itMap->second->mapKey()) {
                //     std::cout << "target key cmp curMap key" << std::endl;
                //     std::cout << (*itList)->line_ << " != " << itMap->second->line_ << std::endl;
                // }
                // getchar();

                std::list<InfoBlock *>::iterator itList2;
                itList2 = (itMap->second)->pos();

                int n1 = std::distance(infoBlockList.begin(), itList);
                int n2 = std::distance(infoBlockList.begin(), itList2);

                int n = n2 - n1 - 1;
                if (n < 0) {
                    itMap = infoBlockMap.erase(itMap);
                    printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
                    continue;
                }

                if (n  > minEqualNb + 50) {
                    ++itMap;
                    continue;
                }

                if (std::distance(itList2, infoBlockList.end()) - 1 < n) {
                    printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
                    ++itMap;
                    continue;
                }

                int i;
                auto itListPos = std::next(itList, n);
                auto itList2Pos = std::next(itList2, n);

                for (i = 1; i < n; ++i, --itListPos, --itList2Pos) {
                    if (**itListPos != **itList2Pos)
                        break;
                }

                if (i < n) {
                    std::cout << "i[" << i << "], n[" << n << "]" << std::endl;
                    printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
                    ++itMap;
                    continue;
                }

                // std::string tmp1, tmp2;
                // itListPos = itList;
                // itList2Pos = itList2;
                // for (int j = 0; j < n ; ++j, ++itListPos) {
                //     tmp1 += (*itListPos)->data();
                // }
                //
                // for (int j = 0; j < n ; ++j, ++itList2Pos) {
                //     tmp2 += (*itList2Pos)->data();
                // }

                // std::cout << "find equal : " << std::endl;
                // std::cout << tmp1 << std::endl;
                // std::cout << "-----------------------" << std::endl;
                // std::cout << tmp2 << std::endl;
                //
                // std::cout << "+++++++++++++++++++++++" << std::endl;
                //
                // std::cout << "del " << n << std::endl;
                // getchar();

                // del [itList2, itList2 + n)
                {
                    auto pos = itList2;
                    --pos;
                    auto lastPos = pos;
                    ++pos;
                    for (int m = 0; m < n; ++m) {


                        infoBlockMap.erase((*pos)->mPos());
                        // auto range = infoBlockMap.equal_range((*pos)->data());
                        // for (auto it = range.first; it != range.second; ) {
                        //     if (it->second == *pos) {
                        //         it = infoBlockMap.erase(it);
                        //         break;
                        //     }
                        //     else {
                        //         ++it;
                        //     }
                        // }

                        std::cout << "del :" << (*pos)->line_ << std::endl;
                        if ((*pos)->repeatCount()) {
                            (*lastPos)->addRepeatCount((*pos)->repeatCount());
                        }

                        pos = infoBlockList.erase(pos);
                    }

                    (*lastPos)->addRepeatCount();

                    hintBegin = itList;
                    // hintBegin = pos;

                    goto __next__;

                }

                ++itMap;
            }

        }

        break;
    }

    std::ofstream outFile(outFileName);

    if (!outFile.is_open()) {
        std::cerr << "Error : open " << outFileName << "failed : " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    for (const auto &it : infoBlockList) {
        outFile.write(it->data().c_str(), it->data().length());
        if (it->repeatCount()) {
            char buf[32];
            snprintf(buf, sizeof(buf), "repeat %d\n", it->repeatCount());
            outFile.write(buf, strlen(buf));
        }
    }

    outFile.flush();
    outFile.close();

    return 0;
}
