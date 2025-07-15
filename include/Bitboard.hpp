#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <fstream>
#include <vector>
#include <iostream>

/*
    The bitboard represents:

    8 | 56 57 58 59 60 61 62 63
    7 | 48 49 50 51 52 53 54 55
    6 | 40 41 42 43 44 45 46 47
    5 | 32 33 34 35 36 37 38 39
    4 | 24 25 26 27 28 29 30 31
    3 | 16 17 18 19 20 21 22 23
    2 | 08 09 10 11 12 13 14 15
    1 | 00 01 02 03 04 05 06 07
    ----------------------------
        A  B  C  D  E  F  G  H
*/

const uint64_t FILEH = 0x8080808080808080;
const uint64_t FILEA = 0x0101010101010101;
const uint64_t RANK1 = 0x00000000000000ff;
const uint64_t RANK8 = 0xff00000000000000;

class Bitboard {
    /* bishop */
    uint64_t c_bishop[64];
    std::vector<uint64_t> bishop_magic_number;
    std::vector<uint32_t> bishop_shift;
    std::vector<uint32_t> bishop_offset;
    std::vector<uint64_t> bishop_cache;
    /* tower - file */
    uint64_t c_file[64];
    std::vector<uint64_t> tower_file_magic_number;
    std::vector<uint32_t> tower_file_shift;
    std::vector<uint32_t> tower_file_offset;
    std::vector<uint64_t> tower_file_cache;
    /* tower - rank */
    uint64_t c_rank[64];
    std::vector<uint64_t> tower_rank_magic_number;
    std::vector<uint32_t> tower_rank_shift;
    std::vector<uint32_t> tower_rank_offset;
    std::vector<uint64_t> tower_rank_cache;
    /* Knight */
    uint64_t c_knight[64];


private:
    void saveOnMemo(std::string filename, std::vector<uint64_t> &number_container, std::vector<uint32_t> &shift_container) {
        std::fstream file;
        file.open(filename.c_str());

        if(!file.is_open()) {
            std::cerr << "Failed when openning " + filename + "\n";
        }

        for(int i=0;i<64;i++) {
            int id;
            uint64_t magic;
            uint32_t shift;

            file >> id >> magic >> shift;
            std::string msg = "file " + filename + " has been failed on line: " + std::to_string(i) + "\n";
            if(id != i) {
                std::cerr << msg;
                assert(id == i);
            }

            number_container.push_back(magic);
            shift_container.push_back(shift);
        }
    }

    std::vector<uint64_t> genMaskOccupancy(uint64_t mask) {
        int bits_on = __builtin_popcountll(mask);

        std::vector<int> bit_pos;
        for(int i=0;i<64;i++) {
            if((mask&(uint64_t(1)<<i)) == 0) continue;
            bit_pos.push_back(i);
        }

        std::vector<uint64_t> ret;

        int configs = (uint64_t(1)<<bits_on);
        for(int m=0;m<configs;m++) {
            uint64_t cmask = 0;
            for(int j=0;j<bit_pos.size();j++) {
                if((m&(uint64_t(1)<<j)) == 0) continue;
                cmask |= (uint64_t(1)<<bit_pos[j]);
            }
            ret.push_back(cmask);
        }

        return ret;
    }

    void computeBishopMoves() {
        saveOnMemo("precompute/magic_numbers/bishop.txt", bishop_magic_number, bishop_shift);

        uint32_t offset = 0;

        for(int i=0;i<64;i++) {
            uint64_t mask = 0;
            uint64_t center = (uint64_t(1)<<i);
    
            mask |= center;

            // << 9
            uint64_t curr_pos = center;
            while((curr_pos&FILEH) == 0 && curr_pos != 0) {
                curr_pos <<= 9;
                mask |= curr_pos;
            }

            curr_pos = center;
            // << 7
            while((curr_pos&FILEA) == 0 && curr_pos != 0) {
                curr_pos <<= 7;
                mask |= curr_pos;
            }

            curr_pos = center;
            // >> 9
            while((curr_pos&FILEA) == 0 && curr_pos != 0) {
                curr_pos >>= 9;
                mask |= curr_pos;
            }

            curr_pos = center;
            // >> 7
            while((curr_pos&FILEH) == 0 && curr_pos != 0) {
                curr_pos >>= 7;
                mask |= curr_pos;
            }

            c_bishop[i] = mask;

            bishop_offset.push_back(offset);

            uint64_t reduced_mask = (mask & ~center & ~FILEA & ~FILEH & ~RANK1 & ~RANK8);
            std::vector<uint64_t> occ_mask = genMaskOccupancy(reduced_mask);

            int occ_mask_size = occ_mask.size();
            for(int j=0;j<occ_mask_size;j++) bishop_cache.push_back(0);

            for(auto omask: occ_mask) {
                uint64_t valid_move = center;

                // << 9
                curr_pos = center;
                while((curr_pos&FILEH) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    curr_pos <<= 9;
                    valid_move |= curr_pos;
                }

                curr_pos = center;
                // << 7
                while((curr_pos&FILEA) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    curr_pos <<= 7;
                    valid_move |= curr_pos;
                }

                curr_pos = center;
                // >> 9
                while((curr_pos&FILEA) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    curr_pos >>= 9;
                    valid_move |= curr_pos;
                }

                curr_pos = center;
                // >> 7
                while((curr_pos&FILEH) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    curr_pos >>= 7;
                    valid_move |= curr_pos;
                }

                // after compute the valid move for an occuped mask, save it in the cache
                int p = ((omask * bishop_magic_number[i]) >> bishop_shift[i]) + bishop_offset[i];
                assert(bishop_cache[p] == 0);
                bishop_cache[p] = valid_move;
            }

            offset += occ_mask_size;
        }
    }

    void computeTowerFileMoves() {
        saveOnMemo("precompute/magic_numbers/file_tower.txt", tower_file_magic_number, tower_file_shift);

        uint32_t offset = 0;

        for(int i=0;i<64;i++) {
            uint64_t mask = 0;
            uint64_t center = (uint64_t(1)<<i);

            mask |= center;

            // << 8
            uint64_t curr_pos = center;
            while((curr_pos & RANK8) == 0 && curr_pos != 0) {
                mask |= curr_pos;
                curr_pos <<= 8;
            }
            // >> 8
            curr_pos = center;
            while((curr_pos & RANK1) == 0 && curr_pos != 0) {
                mask |= curr_pos;
                curr_pos >>= 8;
            }

            c_file[i] = mask;

            tower_file_offset.push_back(offset);
            uint64_t reduced_mask = (mask & ~center & ~RANK1 & ~RANK8);
            std::vector<uint64_t> occ_mask = genMaskOccupancy(reduced_mask);

            int occ_mask_size = occ_mask.size();
            for(int j=0;j<occ_mask_size;j++) tower_file_cache.push_back(0);

            for(auto omask: occ_mask) {
                uint64_t valid_move = center;

                // << 8
                curr_pos = center;
                while((curr_pos & RANK8) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    mask |= curr_pos;
                    curr_pos <<= 8;
                }
                // >> 8
                curr_pos = center;
                while((curr_pos & RANK1) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    mask |= curr_pos;
                    curr_pos >>= 8;
                }

                // after compute the valid move for an occuped mask, save it in the cache
                int p = ((omask * tower_file_magic_number[i]) >> tower_file_shift[i]) + tower_file_offset[i];
                assert(tower_file_cache[p] == 0);
                tower_file_cache[p] = valid_move;
            }

            offset += occ_mask_size;
        }
    }

    void computeTowerRankMoves() {
        saveOnMemo("precompute/magic_numbers/rank_tower.txt", tower_rank_magic_number, tower_rank_shift);

        uint32_t offset = 0;

        for(int i=0;i<64;i++) {
            uint64_t mask = 0;
            uint64_t center = (uint64_t(1)<<i);

            mask |= center;

            // << 1
            uint64_t curr_pos = center;
            while((curr_pos & FILEH) == 0 && curr_pos != 0) {
                mask |= curr_pos;
                curr_pos <<= 1;
            }
            // >> 1
            curr_pos = center;
            while((curr_pos & FILEA) == 0 && curr_pos != 0) {
                mask |= curr_pos;
                curr_pos >>= 1;
            }

            c_rank[i] = mask;

            tower_rank_offset.push_back(offset);
            uint64_t reduced_mask = (mask & ~center & ~FILEA & ~FILEH);
            std::vector<uint64_t> occ_mask = genMaskOccupancy(reduced_mask);

            int occ_mask_size = occ_mask.size();
            for(int j=0;j<occ_mask_size;j++) tower_rank_cache.push_back(0);

            for(auto omask: occ_mask) {
                uint64_t valid_move = center;

                // << 1
                curr_pos = center;
                while((curr_pos & FILEH) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    mask |= curr_pos;
                    curr_pos <<= 1;
                }
                // >> 1
                curr_pos = center;
                while((curr_pos & FILEA) == 0 && curr_pos != 0 && (curr_pos & omask) == 0) {
                    mask |= curr_pos;
                    curr_pos >>= 1;
                }

                // after compute the valid move for an occuped mask, save it in the cache
                int p = ((omask * tower_rank_magic_number[i]) >> tower_rank_shift[i]) + tower_rank_offset[i];
                assert(tower_rank_cache[p] == 0);
                tower_rank_cache[p] = valid_move;
            }

            offset += occ_mask_size;
        }
    }

    void isOut(int x, int y) const {
        return (x < 0 || x > 7 || y < 0 || y > 7);
    }

    void computeKnight() {
        int dx[] = {-1, 1, 2, 2, 1, -1, -2, -2};
        int dy[] = {2, 2, 1, -1, -2, -2, 1, -1};

        for(int x=0;x<8;x++) {
            for(int y=0;y<8;y++) {
                uint64_t mask = 0;
                int p = (x * 8) + y;
                for(int i=0;i<8;i++) {
                    if(isOut(x + dx[i], y + dy[i])) continue;
                    int b = ((x + dx[i]) * 8) + y + dy[i];
                    mask |= (uint64_t(1)<<b);
                }
                c_knight[p] = mask;
            }
        }
    }

    void preprocess() {
        computeBishopMoves();
        computeTowerFileMoves();
        computeTowerRankMoves();
        computeKnight();
    }

public:
    Bitboard() {
        preprocess();
    }

    uint64_t bishop(int cell, uint64_t occupancy) const {
        occupancy &= c_bishop[cell];

        uint64_t center = (uint64_t(1)<<cell);
        uint64_t mask = (occupancy & ~center & ~FILEA & ~FILEH & ~RANK1 & ~RANK8);

        int p = ((mask * bishop_magic_number[cell]) >> bishop_shift[cell]) + bishop_offset[cell];
        return bishop_cache[p];
    }

    uint64_t tower(int cell, uint64_t occupancy) const {
        uint64_t file_occ = (occupancy & c_file[cell]);
        uint64_t rank_occ = (occupancy & c_rank[cell]);

        uint64_t center = (uint64_t(1)<<cell);

        uint64_t fmask = (file_occ & ~center & ~RANK1 & ~RANK8);
        uint64_t rmask = (rank_occ & ~center & ~FILEA & ~FILEH);

        uint64_t mask = 0;

        int p = ((fmask * tower_file_magic_number[cell]) >> tower_file_shift[cell]) + tower_file_offset[cell];
        mask |= tower_file_cache[p];

        p = ((rmask * tower_rank_magic_number[cell]) >> tower_rank_shift[cell]) + tower_rank_offset[cell];
        mask |= tower_rank_cache[p];

        return mask;
    }

    uint64_t knight(int cell) const {
        return c_knight[cell];
    }

    uint64_t king(int cell) const {

    }

    uint64_t pawn(int cell, int side) const {

    }
};

#endif
