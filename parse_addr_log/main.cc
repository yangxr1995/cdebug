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
#include <map>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

int gDelCount;
int gDelLineNb;

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
            return data_ == infoBlock.data_;
            // return type_ == infoBlock.type_ 
            //     && dataHashVal_ == infoBlock.dataHashVal_
            //     && data_ == infoBlock.data_;
        }

        bool operator!=(const InfoBlock &infoBlock) {
            return !(*this == infoBlock);
        }

        void setPos(std::list<InfoBlock *>::iterator pos) {
            pos_ = std::move(pos);
        }

        // void setMPos(std::unordered_multimap<std::string, InfoBlock *>::iterator mpos) {
        void setMPos(std::multimap<std::string, InfoBlock *>::iterator mpos) {
            mpos_ = std::move(mpos);
        }

        // const std::unordered_multimap<std::string, InfoBlock *>::iterator & mPos() const {
        const std::multimap<std::string, InfoBlock *>::iterator & mPos() const {
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
        // std::unordered_multimap<std::string, InfoBlock *>::iterator mpos_;
        std::multimap<std::string, InfoBlock *>::iterator mpos_;
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
    // std::unordered_multimap<std::string, InfoBlock *> infoBlockMap;
    std::multimap<std::string, InfoBlock *> infoBlockMap;

    int minEqualNb = 3;

    // load file
    while(1) {
        std::getline(inFile, line);

        if (inFile.eof()) {
            if (b) {
                infoBlockList.emplace_back(b);
                auto it = infoBlockList.end();
                --it;
                b->setPos(it);
                b->line_ = lineNb;
            }
            break;
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


    auto itList = infoBlockList.end();
    --itList;
    // map
    for (auto itList = infoBlockList.begin(); itList != infoBlockList.end(); ++itList) {
        int n = std::distance(itList, infoBlockList.end());
        if (n < minEqualNb) {
            // std::cout << __LINE__ << ": " << n << " < " << minEqualNb << std::endl;
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

            // 末尾节点没有mapKey
            if ((*itList)->mapKey().empty()) {
                continue;
            }

            auto range = infoBlockMap.equal_range((*itList)->mapKey());
            int itListIndex = std::distance(infoBlockList.begin(), itList);
            std::cout << "itListIndex : " <<  itListIndex  << ", range size : " << std::distance(range.first, range.second) << std::endl;

            for (auto itMap = range.first; itMap != range.second; ) {
                InfoBlock *pInfoBlock = itMap->second;

                if ((*itList)->line_ >= pInfoBlock->line_) {
                    itMap = infoBlockMap.erase(itMap);
                    // std::cout << "continue " << __LINE__ << std::endl;
                    continue;
                }

                int itListIndex2 = std::distance(infoBlockList.begin(), pInfoBlock->pos());
                int n = itListIndex2 - itListIndex;

                if (n < minEqualNb) {
                    ++itMap;
                    continue;
                }

                if (n > 100) {
                    break;
                }

                // std::cout << "itListIndex2 - itListIndex : " << n << std::endl;

                if (std::distance(pInfoBlock->pos(), infoBlockList.end()) < n) {
                    // std::cout << "continue " << __LINE__ << std::endl;
                    ++itMap;
                    continue;
                }

                auto pos1 = std::next(itList, n - 1);
                auto pos2 = std::next(pInfoBlock->pos(), n - 1);
                size_t i;
                for (i = 0; i < n; i++, --pos1, --pos2) {
                    if ((*pos1)->data() != (*pos2)->data()) {
                        break;
                    }
                }

                if (i == n) {
                    gDelCount++;
                    // std::cout << "find equal at " << (*itList)->line_ << std::endl;

                    auto pos = std::next(itList, n - 1);
                    (*pos)->addRepeatCount();

                    pos = pInfoBlock->pos();

                    // std::cout << "find equal2 at " << (*pos)->line_ << std::endl;

                    for (int i = 0; i < n; ++i) {
                        // std::cout << "del " << (*pos)->line_ << std::endl;
                        infoBlockMap.erase((*pos)->mPos());
                        pos = infoBlockList.erase(pos);
                        gDelLineNb++;
                    }

                    hintBegin = itList;

                    goto __next__;
                }
                else {
                    // std::cout << "continue " << __LINE__ << std::endl;
                }

                ++itMap;
            }

        }

        break;
    }

    std::cout << "gDelCount : " << gDelCount << std::endl;
    std::cout << "gDelLineNb : " << gDelLineNb * 3 << std::endl;

    std::ofstream outFile(outFileName);

    if (!outFile.is_open()) {
        std::cerr << "Error : open " << outFileName << "failed : " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    for (const auto &it : infoBlockList) {
        outFile.write(it->data().c_str(), it->data().length());
        if (it->repeatCount()) {
            char buf[32];
            snprintf(buf, sizeof(buf), "repeat ...\n");
            outFile.write(buf, strlen(buf));
        }
    }

    outFile.flush();
    outFile.close();

    return 0;
}
