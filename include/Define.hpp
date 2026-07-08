#ifndef DEFINE_H
#define DEFINE_H

const uint64_t FILEH = 0x8080808080808080;
const uint64_t FILEA = 0x0101010101010101;
const uint64_t RANK1 = 0x00000000000000ff;
const uint64_t RANK8 = 0xff00000000000000;

typedef std::pair<int, int> i2;
typedef std::pair<i2, int> i3;
typedef std::pair<i2, i2> i4;
typedef std::pair<i4, int> i5;

typedef std::vector<i2> vi2;
typedef std::vector<i3> vi3;
typedef std::vector<i4> vi4;
typedef std::vector<i5> vi5;

#define WR 0
#define WN 1
#define WB 2
#define WQ 3
#define WK 4
#define WP 5
#define BR 6
#define BN 7
#define BB 8
#define BQ 9
#define BK 10
#define BP 11
#define OUT 12
#define EMPTY 13
#define DRAW 14
#define CHECKMATE 15
#define ALIVE 16

// pieces_counter sub-slots tracking bishop square color, kept apart from
// the WR..BP material indices above so they can never collide with them
#define WB_LIGHT 20
#define WB_DARK 21
#define BB_LIGHT 22
#define BB_DARK 23

inline int getColor(int piece) {
    if(piece > 11) return -1;
    if(piece < 6) return 0;
    return 1;
}

inline bool isWhite(int piece) {
    return getColor(piece) == 0;
}

inline bool isBlack(int piece) {
    return getColor(piece) == 1;
}

inline bool isQueen(int piece) {
    return (piece == 3 || piece == 9);
}

inline bool isBishop(int piece) {
    return (piece == 2 || piece == 8);
}

inline bool isRook(int piece) {
    return (piece == 0 || piece == 6);
}

inline bool isKnight(int piece) {
    return (piece == 1 || piece == 7);
}

inline bool isKing(int piece) {
    return (piece == 4 || piece == 10);
}

inline bool isPawn(int piece) {
    return (piece == 5 || piece == 11);
}

inline std::string getPieceName(int piece) {
    if(piece == WR) return "wr";
    if(piece == WN) return "wn";
    if(piece == WB) return "wb";
    if(piece == WQ) return "wq";
    if(piece == WK) return "wk";
    if(piece == WP) return "wp";
    if(piece == BR) return "br";
    if(piece == BN) return "bn";
    if(piece == BB) return "bb";
    if(piece == BQ) return "bq";
    if(piece == BK) return "bk";
    if(piece == BP) return "bp";
    return "";
}

#endif
