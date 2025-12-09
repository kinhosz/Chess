#ifndef DEFINE_H
#define DEFINE_H

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
