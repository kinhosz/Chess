#include <bits/stdc++.h>
using namespace std;

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

const uint64_t COLA = 0x0101010101010101;
const uint64_t COLH = 0x8080808080808080;
const uint64_t ROW1 = 0x00000000000000ff;
const uint64_t ROW8 = 0xff00000000000000;

void printMask(uint64_t mask) {
  string x = "";
  for(int i=63;i>=0;i--) {
    uint64_t b = 1;

    x += ((mask&(b<<i)) == 0 ? '0' : 'X');

    if(i%8 == 0) {
      reverse(x.begin(), x.end());
      cerr << x << "\n";
      x = "";
    }
  }
}

uint64_t find_magic_number(int id, int shift, vector<uint64_t> &occ_mask) {
  bool found = false;

  int cache_size = occ_mask.size();

  vector<bool> hit(cache_size);
  int tries = 1;
  uint64_t magic;

  cerr << "----------------------\n";
  cerr << "Starting the magic...\n";
  cerr << "----------------------\n";

  int PART = 10000;
  int maxi = 0;

  while(!found) {
    magic = uniform_int_distribution<uint64_t>()(rng);

    for(int i=0;i<cache_size;i++) hit[i] = false;

    if(tries % PART == 0) cerr << "[Magic for " << id << "] #" << tries << ": ";

    found = true;
    for(int i=0;i<occ_mask.size();i++) {
      uint64_t pos = (occ_mask[i] * magic) >> shift;;

      if(hit[pos]) {
        found = false;
        if(tries % PART == 0) cerr << "Failed on " << maxi << "/" << cache_size << "\n";
        maxi = max(maxi, i+1);
        break;
      }

      hit[pos] = true;
    }

    if(found) {
      cerr << " Success!!!!!! ========" << magic << "========\n\n";
    }

    tries++;
  }

  return magic;
}

void store(int id, uint64_t magic, int shift) {
  cout << id << " " << magic << " " << shift << "\n";
  cerr << "-----------------------------------------\n";
  cerr << "id:    " << id << "\n";
  cerr << "magic: " << magic << "\n";
  cerr << "shift: " << shift << "\n\n";
}

int main() {
  ios::sync_with_stdio(0);
  cin.tie(0);

  freopen("bishop_magic_number.txt", "w", stdout);

  uint64_t bishop_pos;

  vector<uint64_t> masks;

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

  for(int i=0;i<64;i++) {
    uint64_t bit = 1;
    bishop_pos = (bit<<i);

    uint64_t mask = bishop_pos;

    // << 9
    uint64_t curr_pos = bishop_pos;
    while((curr_pos&COLH) == 0 && curr_pos != 0) {
      curr_pos <<= 9;
      mask |= curr_pos;
    }

    curr_pos = bishop_pos;
    // << 7
    while((curr_pos&COLA) == 0 && curr_pos != 0) {
      curr_pos <<= 7;
      mask |= curr_pos;
    }

    curr_pos = bishop_pos;
    // >> 9
    while((curr_pos&COLA) == 0 && curr_pos != 0) {
      curr_pos >>= 9;
      mask |= curr_pos;
    }

    curr_pos = bishop_pos;
    // >> 7
    while((curr_pos&COLH) == 0 && curr_pos != 0) {
      curr_pos >>= 7;
      mask |= curr_pos;
    }

    masks.push_back(mask);
  }

  std::cerr << "Generated all clean masks\n";

  for(int i=0;i<masks.size();i++) {
    cerr << "--------------------\n";
    printMask(masks[i]);

		uint64_t center = (uint64_t(1)<<i);
    uint64_t mask = masks[i];
    uint64_t mask_no_border_no_center = (mask & ~COLA & ~COLH & ~ROW1 & ~ROW8 & ~center);

    int bits_on = __builtin_popcountll(mask_no_border_no_center);
    vector<int> bit_pos;
    
    for(int b=0;b<64;b++) {
      if((mask_no_border_no_center&(uint64_t(1)<<b)) != 0) bit_pos.push_back(b);
    }
    assert(bit_pos.size() == bits_on);

    vector<uint64_t> occ_mask;
    int configs = (uint64_t(1)<<bits_on);
    for(int m=0;m<configs;m++) {
      uint64_t curr_mask = 0;

      for(int j=0;j<bit_pos.size();j++) {
        int b = bit_pos[j];

        if((m&(uint64_t(1)<<j)) == 0) continue;

        curr_mask |= (uint64_t(1)<<b);
      }

      occ_mask.push_back(curr_mask);
    }

    uint64_t magic = find_magic_number(i, 64 - bits_on, occ_mask);
    store(i, magic, 64-bits_on);
  }
}