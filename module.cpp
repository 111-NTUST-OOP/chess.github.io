/****************************************************************************
 * File: module.cpp
 * Author: 朱昱綸 (B11115022@mail.ntust.edu.tw)
 *         張庭嘉 (B11115027@mail.ntust.edu.tw)
 *         顏毓德 (B11115038@mail.ntust.edu.tw)
 *         黃雋祐 (B11115048@mail.ntust.edu.tw)
 * Create Date: May 14, 2023
 * Update Date: May 15, 2023
 * Description: This file contains string-based FEN processing functions
                designed to be compiled to a stateless WASM module with
                minimum bindings.
****************************************************************************/

#ifdef EMSCRIPTEN
  #include <emscripten/bind.h>
  using namespace emscripten;
#endif

#include <string>
#include <algorithm>
#include <numeric>
#include <execution>
#include <vector>
#include <regex>

enum FEN_DATAROW { RANK8, RANK7, RANK6, RANK5, RANK4, RANK3, RANK2, RANK1, ACTIVE_COLOR, CASTLING_RIGHTS, EN_PASSANT, HALFMOVE_CLOCK, FULLMOVE_NUMBER, BLACKKING_CRD, WHITEKING_CRD, SIZE };
const std::regex FEN_PATTERN("^([rnbqkpRNBQKP1-8]{1,8}\\/){7}[rnbqkpRNBQKP1-8]{1,8} [wb] (-|[kqKQ]{1,4}) (-|[a-h][1-8]) \\d+ \\d+$");
const std::regex MOVE_PATTERN("^[a-h][1-8][a-h][1-8](|[nbrqNBRQ])$");


struct Dir { int x; int y; };
  
struct Pos {
  size_t x;
  size_t y;
  bool operator==(const Pos &rhs) const { return x == rhs.x && y == rhs.y; }
};

// Intent: Check if a coordiante is valid (coordinates from "a1" to "h8" are valid)
// Pre: None
// Post: None
bool isValidCrd(const std::string &crd) {
  return crd.size() == 2 && crd[0] >= 'a' && crd[0] <= 'h' && crd[1] >= '1' && crd[1] <= '8';
}

// Intent: Convert a coordinate to Pos of x and y used for array indexing, y is inverted
// Pre: None
// Post: bool(result) == false if the input is invalid
Pos crd2pos(const std::string &crd) {
  return isValidCrd(crd) ? Pos{size_t(crd[0] - 'a'), size_t('8' - crd[1])} : Pos{8, 8};
}

// Intent: Convert vectors returned by parseFEN back into a FEN string
// Pre: None
// Post: result.size() == 0 if the input is invalid
std::string data2fen(const std::vector<std::string> &data) {
  if (data.size() != SIZE)
    return "";
  
  std::string result;
  
  for (size_t y = RANK8; y <= RANK1; ++y) {
    size_t spaces = 0;
    for (const char &c : data[y]) {
      if (c == ' ') {
        ++spaces;
      } else {
        if (spaces) {
          result += std::to_string(spaces);
          spaces = 0;
        }
        result += c;
      }
    }
    if (spaces)
      result += std::to_string(spaces);
    result += '/';
  }
  result.back() = ' ';
  
  result += data[ACTIVE_COLOR] + ' ' + data[CASTLING_RIGHTS] + ' ' + data[EN_PASSANT]
         + ' ' + data[HALFMOVE_CLOCK] + ' ' + data[FULLMOVE_NUMBER];
  
  return result;
}

// Intent: Increment a numeric string by 1
// Pre: None
// Post: result.size() == 0 if inputs are invalid
std::string incrementStr(const std::string &str) {
  try {
    return std::to_string(std::stoi(str) + 1);
  } catch (...) { // std::invalid_argument and std::out_of_range
    return "";
  }
}

// Intent: convert array index x and y into a string, y is inverted: (0,0) -> "a8"  (2, 3) -> "c5"  (7,7) -> "h1"
// Pre: None
// Post: result.size() == 0 if inputs are invalid
std::string xy2crd(const size_t &x, const size_t &y) {
  return std::max(x, y) <= 7 ? char('a' + x) + std::to_string(8 - y) : "";
}

// Intent: Parse FEN string
// Pre: None
// Post: result.size() == 0 if the input is invalid
std::vector<std::string> parseFEN(const std::string &fen) {
  // check the string format
  if (!std::regex_match(fen, FEN_PATTERN))
    return {};
  
  std::vector<std::string> result(SIZE); // may actually throw std::bad_alloc but very unlikely
  auto it = fen.begin();
  
  // parse the data
  for (size_t y = RANK8; it != fen.end(); ++it) {
    if (*it == '/' || *it == ' ') { // split the string
      // check if each row in chess board is exactly 8 chars long
      if (y <= RANK1 && result[y].size() != 8)
        return {};
      ++y;
    } else if (y <= RANK1 && isdigit(*it)) { // add spaces to chess board
      result[y] += std::string(*it - '0', ' ');
    } else { // add char to datarows
      if (y <= RANK1 && tolower(*it) == 'k')
        result[islower(*it) ? BLACKKING_CRD : WHITEKING_CRD] += xy2crd(result[y].size(), y);
      result[y] += *it;
    }
  }
  
  // check if each side has exactly 1 king
  if (result[BLACKKING_CRD].size() != 2 || result[WHITEKING_CRD].size() != 2)
    return {};
  
  return result;
}

// Intent: Get the FEN after making a move, the move formatted as such:
//           "e2e4"   : e2 to e4
//           "e7e8Q" : e7 to e8 and promote to white queen
//           "e5f6"   : e5 to f6 (or en passant)
//           "e1g1"   : e1 to g1 (or castle kingside)
// Pre: Make sure the move is legal, as this function ignores most chess rules
// Post: result.size() == 0 if the FEN is invalid or if the move is out of bounds
std::string getNextFEN(const std::string &fen, const std::string &mov) {
  // invalid move format
  if (!std::regex_match(mov, MOVE_PATTERN))
    return "";
  
  // invalid FEN
  auto data = parseFEN(fen);
  if (data.size() != SIZE)
    return "";
  
  auto [sx, sy] = crd2pos(mov.substr(0, 2));
  auto [tx, ty] = crd2pos(mov.substr(2, 2));
  int dx = int(tx) - int(sx);
  int dy = int(ty) - int(sy);
  char &sp = data[sy][sx];
  char &tp = data[ty][tx];
  char promotion = mov.size() == 5 ? mov.back() : 0;
  
  // castling
  if (tolower(sp) == 'k') {
    if (dx == 2) // castle kingside
      std::swap(data[sy][7], data[sy][sx + 1]);
    else if (dx == -2) // castle queenside
      std::swap(data[sy][0], data[sy][sx - 1]);
  }
  
  // en passant
  if (tolower(sp) == 'p' && Pos{tx, ty} == crd2pos(data[EN_PASSANT]))
    data[sy][tx] = ' ';
  
  auto removeCastlingRights = [&castling = data[CASTLING_RIGHTS]](const std::string &rem) {
    for (char c : rem) {
      auto pos = castling.find(c);
      if (pos != std::string::npos)
        castling.erase(pos, 1);
    }
    if (castling.size() == 0)
      castling = "-";
  };
  
  // remove castling rights
  if (sp == 'r' || tp == 'r')
    removeCastlingRights(tx == 7 ? "k" : "q");
  else if (sp == 'R' || tp == 'R')
    removeCastlingRights(tx == 7 ? "K" : "Q");
  if (sp == 'k')
    removeCastlingRights("kq");
  else if (sp == 'K')
    removeCastlingRights("KQ");
  
  // update en passant target square
  data[EN_PASSANT] = (tolower(sp) == 'p' && abs(dy) == 2) ? xy2crd(sx, sy + dy / 2) : "-";
  
  // reset or increment the halfmove clock
  data[HALFMOVE_CLOCK] = (tolower(sp) == 'p' || tp != ' ') ? "0" : incrementStr(data[HALFMOVE_CLOCK]);
  
  // increment the fullmove counter and switch active color
  if (data[ACTIVE_COLOR] == "b") {
    data[FULLMOVE_NUMBER] = incrementStr(data[FULLMOVE_NUMBER]);
    data[ACTIVE_COLOR] = 'w';
  } else {
    data[ACTIVE_COLOR] = 'b';
  }
  
  // move and promote the piece
  tp = promotion ? promotion : sp;
  sp = ' ';
  
  return data2fen(data);
}

// Intent: Convert FEN to '\0'-seperated string of html class names
// Pre: None
// Post: None
std::string fenToHtmlClassNames(const std::string &fen) {
  std::string pieces = "";
  auto data = parseFEN(fen);
  
  // invalid FEN
  if (data.size() != SIZE)
    return "";
  
  // concatenate chess board to a string
  pieces = std::accumulate(data.begin(), data.begin() + 8, std::string{});
  
  // replace each piece with its html class name + ' '
  std::unordered_map<char, std::string> classNames{
    {' ', "empty-square"},
    {'k', "piece black-king"},
    {'p', "piece black-pawn"},
    {'n', "piece black-knight"},
    {'b', "piece black-bishop"},
    {'r', "piece black-rook"},
    {'q', "piece black-queen"},
    {'K', "piece white-king"},
    {'P', "piece white-pawn"},
    {'N', "piece white-knight"},
    {'B', "piece white-bishop"},
    {'R', "piece white-rook"},
    {'Q', "piece white-queen"},
  };
  
  std::string result = "";
  
  for (const char &p : pieces)
    result += classNames[p] + '\0';
  result.pop_back();
  return result;
}

// Intent: Get valid moves that start from the source coordinate
// Pre: None
// Post: The return value is a '\0'-seperated string of valid target squares
std::string getValidTargetSquares(const std::string &fen, const std::string &src, const bool showWhoIsInCheck = false) {
  std::vector<Pos> targetSquares;
  auto [sx, sy] = crd2pos(src);
  
  // invalid coordinate
  if (std::max(sx, sy) > 7)
    return "";
  
  // invalid FEN
  auto data = parseFEN(fen);
  if (data.size() != SIZE)
    return "";
  
  auto [kx, ky] = crd2pos(data[ACTIVE_COLOR] == "w" ? data[WHITEKING_CRD] : data[BLACKKING_CRD]);
  
  auto isBlocked = [&data](const size_t &x, const size_t &y) {
    return std::max(x, y) > 7 || (data[ACTIVE_COLOR] == "w" ? isupper(data[y][x]) : islower(data[y][x]));
  };
  
  auto addMove = [&](const size_t &x, const size_t &y) {
    targetSquares.push_back({x, y});
  };
  
  auto addMoves = [&](const Dir &dir) {
    auto [dirX, dirY] = dir;
    size_t x = sx, y = sy;
    while (!isBlocked(x += dirX, y += dirY)) {
      addMove(x, y);
      if (data[y][x] != ' ')
        break;
    }
  };
  
  auto isAttackedByEnemy = [&](const size_t &x, const size_t &y) {
    auto isAttackedFromBy = [&](int dirX, int dirY, const std::string &attackers, size_t range = 7) {
      size_t curX = x, curY = y;
      while (range-- && !isBlocked(curX += dirX, curY += dirY)) {
        if (attackers.find(data[curY][curX]) != std::string::npos)
          return true;
        if (data[curY][curX] != ' ')
          break;
      }
      return false;
    };
    return isAttackedFromBy(1, 1, "bqBQ") || isAttackedFromBy(1, -1, "bqBQ") || isAttackedFromBy(-1, -1, "bqBQ") || isAttackedFromBy(-1, 1, "bqBQ") ||
           isAttackedFromBy(1, 0, "rqRQ") || isAttackedFromBy(0, 1, "rqRQ") || isAttackedFromBy(-1, 0, "rqRQ") || isAttackedFromBy(0, -1, "rqRQ") ||
           isAttackedFromBy(-2, -1, "nN", 1) || isAttackedFromBy(-1, -2, "nN", 1) || isAttackedFromBy(1, -2, "nN", 1) || isAttackedFromBy(2, -1, "nN", 1) ||
           isAttackedFromBy(2, 1, "nN", 1) || isAttackedFromBy(1, 2, "nN", 1) || isAttackedFromBy(-1, 2, "nN", 1) || isAttackedFromBy(-2, 1, "nN", 1) ||
           isAttackedFromBy(1, 1, "pkPK", 1) || isAttackedFromBy(-1, 1, "pkPK", 1) || isAttackedFromBy(1, -1, "pkpK", 1) || isAttackedFromBy(-1, -1, "pkpK", 1) ||
           isAttackedFromBy(1, 0, "kK", 1) || isAttackedFromBy(0, 1, "kK", 1) || isAttackedFromBy(-1, 0, "kK", 1) || isAttackedFromBy(0, -1, "kK", 1)
           ;
  };
  
  // Moving enemy pieces or empty squares is not allowed
  if (!isBlocked(sx, sy)) {
    std::string result;
    if (showWhoIsInCheck && isAttackedByEnemy(kx, ky)) {
      result = data[ACTIVE_COLOR] == "w" ? "White" : "Black";
      result += " is in check";
    }
    return result;
  }
  
  // generate pseudo-legal moves
  char sp = data[sy][sx];
  char sptype = tolower(sp);
  
  if (sptype == 'k') {
    for (auto [x, y] : {Pos{sx + 1, sy}, {sx + 1, sy + 1}, {sx, sy + 1}, {sx - 1, sy + 1}, {sx - 1, sy}, {sx - 1, sy - 1}, {sx, sy - 1}, {sx + 1, sy - 1}})
      if (!isBlocked(x, y))
        addMove(x, y);
    
    if (data[CASTLING_RIGHTS].find(islower(sp) ? 'k' : 'K') != std::string::npos && !isBlocked(sx + 1, sy) && !isBlocked(sx + 2, sy)
      && !isAttackedByEnemy(sx, sy) && !isAttackedByEnemy(sx + 1, sy) && !isAttackedByEnemy(sx + 2, sy))
    {
      addMove(sx + 2, sy);
    }
    
    if (data[CASTLING_RIGHTS].find(islower(sp) ? 'q' : 'Q') != std::string::npos && !isBlocked(sx - 1, sy) && !isBlocked(sx - 2, sy) && !isBlocked(sx - 3, sy)
      && !isAttackedByEnemy(sx, sy) && !isAttackedByEnemy(sx - 1, sy) && !isAttackedByEnemy(sx - 2, sy))
    {
      addMove(sx - 2, sy);
    }
  }
  else if (sp == 'p') {  
    auto ePos = crd2pos(data[EN_PASSANT]);
    
    if (!isBlocked(sx + 1, sy + 1) && data[sy + 1][sx + 1] != ' ' || Pos{sx + 1, sy + 1} == ePos)
      addMove(sx + 1, sy + 1);
    
    if (!isBlocked(sx - 1, sy + 1) && data[sy + 1][sx - 1] != ' ' || Pos{sx - 1, sy + 1} == ePos)
      addMove(sx - 1, sy + 1);
    
    if (!isBlocked(sx, sy + 1) && data[sy + 1][sx] == ' ') {
      addMove(sx, sy + 1);
      if (sy == 1 && !isBlocked(sx, sy + 2) && data[sy + 2][sx] == ' ') {
        addMove(sx, sy + 2);
      }
    }
  }
  else if (sp == 'P') {
    auto ePos = crd2pos(data[EN_PASSANT]);
    
    if (!isBlocked(sx + 1, sy - 1) && data[sy - 1][sx + 1] != ' ' || Pos{sx + 1, sy - 1} == ePos)
      addMove(sx + 1, sy - 1);
    
    if (!isBlocked(sx - 1, sy - 1) && data[sy - 1][sx - 1] != ' ' || Pos{sx - 1, sy - 1} == ePos)
      addMove(sx - 1, sy - 1);
    
    if (!isBlocked(sx, sy - 1) && data[sy - 1][sx] == ' ') {
      addMove(sx, sy - 1);
      if (sy == 6 && !isBlocked(sx, sy - 2) && data[sy - 2][sx] == ' ') {
        addMove(sx, sy - 2);
      }
    }
  }
  else if (sptype == 'n') {
    for (auto [x, y] : {Pos{sx - 2, sy - 1}, {sx - 1, sy - 2}, {sx + 1, sy - 2}, {sx + 2, sy - 1}, {sx + 2, sy + 1}, {sx + 1, sy + 2}, {sx - 1, sy + 2}, {sx - 2, sy + 1}}) {
      if (!isBlocked(x, y))
        addMove(x, y);
    }
  }
  else if (sptype == 'b') {
    for (auto dir : {Dir{1, 1}, {1, -1}, {-1, 1}, {-1, -1}})
      addMoves(dir);
  }
  else if (sptype == 'r') {
    for (auto dir : {Dir{1, 0}, {0, 1}, {-1, 0}, {0, -1}})
      addMoves(dir);
  }
  else if (sptype == 'q') {
    for (auto dir : {Dir{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}})
      addMoves(dir);
  }
  
  // filter illegal moves that leave the king in check & form the result string
  std::string result = "";
  
  for (const Pos &p : targetSquares) {
    auto [tx, ty] = p;
    char tp = data[ty][tx];
    
    // move the piece
    data[sy][sx] = ' ';
    data[ty][tx] = sp;
    
    // check if king is attacked after moving
    if ((sptype == 'k') ? (!isAttackedByEnemy(tx, ty)) : (!isAttackedByEnemy(kx, ky)))
      result += xy2crd(tx, ty) + '\0';
    
    // undo the move
    data[sy][sx] = sp;
    data[ty][tx] = tp;
  }
  
  // remove trailing '\0' character
  if (result.size()) {
    result.pop_back();
  } else if (showWhoIsInCheck && isAttackedByEnemy(kx, ky)) {
    result = data[ACTIVE_COLOR] == "w" ? "White" : "Black";
    result += " is in check";
  }
  
  return result;
}

// Intent: Check if a move is valid
// Pre: None
// Post: None
bool isValidMove(const std::string &fen, const std::string &move) {
  if (!std::regex_match(move, MOVE_PATTERN))
    return false;
  
  std::string source = move.substr(0, 2);
  std::string validSquares = getValidTargetSquares(fen, source);
  std::string target = move.substr(2, 2);
  
  return validSquares.find(target) != std::string::npos;
}

// Intent: Get the game state, which is one of the following:
//           1. "White to move"
//           2. "Black to move"
//           3. "Checkmate: White wins"
//           4. "Checkmate: Black wins"
//           5. "Stalemate: Draw"
//           6. "Invalid FEN"
// Pre: None
// Post: None
std::string getGameState(const std::string &fen) {
  auto data = parseFEN(fen);
  if (data.size() != SIZE)
    return "Invalid FEN";
  
  for (size_t y = 0; y < 8; ++y) {
    for (size_t x = 0; x < 8; ++x) {
      if (getValidTargetSquares(fen, xy2crd(x, y)).size()) {
        return data[ACTIVE_COLOR] == "w" ? "White to move" : "Black to move";
      }
    }
  }
  
  // Check if king is in check
  auto res = getValidTargetSquares(fen, xy2crd(0, 0), true);
  if (res == "White is in check")
      return "Checkmate: Black wins";
  if (res == "Black is in check")
    return "Checkmate: White wins";
  
  return "Stalemate: Draw";
}

#ifdef EMSCRIPTEN // em++ function bindings

EMSCRIPTEN_BINDINGS(chessModule) {
  function("getGameState", &getGameState);
  function("isValidMove", &isValidMove);
  function("getValidTargetSquares", optional_override([](const std::string& fen, const std::string &crd) {
    return getValidTargetSquares(fen, crd);
  }), allow_raw_pointers());
  function("getNextFEN", &getNextFEN);
  function("fenToHtmlClassNames", &fenToHtmlClassNames);
}

#endif // ifdef EMSCRIPTEN

#ifndef EMSCRIPTEN // debug

/*
mid game benchmark, white to play r3kb1r/1pBnp1pp/p4p2/1N1n1b2/2BP4/5NP1/P4P1P/R1R3K1 w kq - 0 17
initial state rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 1 1
checkmate in 6 8/8/8/8/8/4K3/5Q2/7k w - - 11 56
checkmate in 1 8/8/8/8/8/6K1/4Q3/6k1 w - - 21 61
github issue 4, castling (slow search) r6r/p3kp1p/4np2/1Bb5/3p4/P4N2/1P3PPP/R3K2R w KQ - 2 18
github issue 4, no castling r6r/p3kp1p/4np2/1Bb5/3p4/P4N2/1P3PPP/R3K2R w - - 2 18
mid game benchmark, black to play r3kb1r/ppBnp1pp/5p2/1N1n1b2/2BP4/5NP1/P4P1P/R1R3K1 b kq - 1 16
late game, rook pins bishop 4k3/4n3/8/3N1R2/4R2p/7P/1r3BK1/8 b - - 6 42
end game, impossible pawn 4p3/8/8/8/8/k6P/6K1/8 b - - 6 42
en passant rn1qkbnr/p1p1pppp/8/1pPp4/3P1B2/8/PP2PPPP/Rb1QKBNR w KQkq b6 0 5
too many pieces rnbqkbnr/pppppppp/nnnnnnnn/PPPPPPPP/pppppppp/NNNNNNNN/PPPPPPPP/RNBQKBNR w KQkq - 1 1
en passant rnb1r1k1/ppp2ppp/8/8/2PN4/2Nn4/P3BPPP/R3K2R w KQ - 5 14
github issue 5, castling r3k2Q/pp3p1p/3qp3/2pp2N1/3P4/4PP2/PP1K2PP/nNB4R b k - 0 15
late-ish 8/p2P1N2/8/4p2k/1p2P3/1P1b2pK/P6P/n7 w - - 0 33
queening opportunities 4kb1R/1p1np1P1/2B2p2/1N1P1b2/8/5NK1/p3rP1p/8 w - - 0 31
mate in 1 5k2/8/5K2/4Q3/5P2/8/8/8 w - - 3 61
incomplete FEN, stalemate rn2k1nr/pp4pp/3p4/q1pP4/P1P2p1b/1b2pPRP/1P1NP1PQ/2B1KBNR w Kkq -
stalemate 5bnr/4p1pq/4Qpkr/7p/2P4P/8/PP1PPPP1/RNB1KBNR b KQ - 0 10
stalemate in 1 (Qxe6) 5bnr/4p1pq/2Q1ppkr/7p/2P4P/8/PP1PPPP1/RNB1KBNR w KQ - 0 10
rook & king 8/7K/8/8/8/8/R7/7k w - - 0 1
zugzwang 8/8/p1p5/1p5p/1P5p/8/PPP2K1p/4R1rk w - - 0 1
earlyish rnq1nrk1/pp3pbp/6p1/3p4/3P4/5N2/PP2BPPP/R1BQK2R w KQ -
checkmate in 2 4kb2/3r1p2/2R3p1/6B1/p6P/P3p1P1/P7/5K2 w - - 0 36
“leonid's position” q2k2q1/2nqn2b/1n1P1n1b/2rnr2Q/1NQ1QN1Q/3Q3B/2RQR2B/Q2K2Q1 w - -
insufficient material 8/7K/8/8/8/8/N7/7k w - - 40 40
sufficient material - knight 8/7K/8/5n2/8/8/N7/7k w - - 40 40
insufficient material - bishops 8/6BK/7B/6b1/7B/8/1B6/7k w - - 40 40
sufficient material - opposing bishops 8/6BK/7B/6b1/7B/8/B7/7k w - - 40 40
sufficient material 8/7K/8/8/7B/8/N7/7k w - - 40 40
castling; various checks r3k2r/p4p1p/4np2/1Bb5/3p4/P3nN2/1P3PPP/R3K2R w KQkq - 2 18
Adrian Dușa's en-passant position r2k3B/ppp2p1p/7n/3p4/8/1P3N2/P4qP1/4RRK1 w q - 0 17
Adrian Dușa's position, no castling r2k3B/ppp2p1p/7n/3p4/8/1P3N2/P4qP1/4RRK1 w - - 0 17
Adrian Dușa's continuation r7/pp1k1p1p/5B1n/2pp4/8/1P3N2/P4RP1/4R1K1 w q - 3 19
blatantly false castling claims 8/7K/8/8/8/8/q7/7k w KQq - 40 40
*/


#include "debug.h"
int main() {
  println("===== TEST (module.cpp) =====");
  println();
  
  std::string fen1 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  std::string fen2 = "rnbqkbnr/pp1p1ppp/2p5/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
  std::string fen3 = "r2qkb1r/ppp1pppp/2n2n2/3p4/2PP1B2/2N5/PP2PPPP/R2QKBNR w KQkq d6 0 5";
  std::string fen4 = "r1bqkbnr/ppp2ppp/2n1p3/3p4/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq d6 1 3";
  std::string fen5 = "r1b1kbnr/ppp2p2/2B3pp/6N1/8/6P1/PPP1PP1P/RNB2RK1 b - - 0 10";
  std::string fen6 = "8/5R2/2pk1p2/8/1P6/2KBr3/PP6/8 b - - 0 29";
  
  println(xy2crd(0, -1));
  println(xy2crd(0, 0));
  println(xy2crd(0, 1));
  println(xy2crd(0, 2));
  println(xy2crd(0, 3));
  println(xy2crd(0, 4));
  println(xy2crd(0, 5));
  println(xy2crd(0, 6));
  println(xy2crd(0, 7));
  println(xy2crd(0, 8));
  println(crd2pos(xy2crd(0, -1)));
  println(crd2pos(xy2crd(0, 0)));
  println(crd2pos(xy2crd(0, 1)));
  println(crd2pos(xy2crd(0, 2)));
  println(crd2pos(xy2crd(0, 3)));
  println(crd2pos(xy2crd(0, 4)));
  println(crd2pos(xy2crd(0, 5)));
  println(crd2pos(xy2crd(0, 6)));
  println(crd2pos(xy2crd(0, 7)));
  println(crd2pos(xy2crd(0, 8)));
  
  
  println(fenToHtmlClassNames(fen1));
  println();
  
  for (const std::string &mov : {"e2e4Q", "e7e5", "h2h4", "g1f3", "b8c6", "f1b5", "d7d6", "e1g1", "c8g4"})
    println(mov.substr(0, 2), getValidTargetSquares(fen1, mov.substr(0, 2)));
  println();
  
  for (const std::string &mov : {"e7e6", "d2d4", "e6d5", "e4d5", "g8f6", "f1d3", "b7b6", "c1e3", "d7d6", "e3f4", "c8b7"})
    println(mov.substr(0, 2), getValidTargetSquares(fen2, mov.substr(0, 2)));
  println();
  
  for (const std::string &mov : {"d4d5", "e6d5", "c4d5", "d8d5", "b1c3", "g8f6", "g2g3", "e7e5", "f1h3", "h7h6"})
    println(mov.substr(0, 2), getValidTargetSquares(fen3, mov.substr(0, 2)));
  println();
  
  for (const std::string &mov : {"d4d5", "e6d5", "g1f3", "d5c4", "c1g5", "c7c6", "d1d3", "b8c6", "h2h3", "g8f6"})
    println(mov.substr(0, 2), getValidTargetSquares(fen4, mov.substr(0, 2)));
  println();
  
  
  
  
  
  
}

#endif // ifndef EMSCRIPTEN
