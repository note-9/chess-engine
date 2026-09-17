#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

enum {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum {
  white,
  black,
  both
};

enum {
  rook,
  bishop
};

enum {
  wk = 1,
  wq = 2,
  bk = 4,
  bq = 8
};

enum {
  P, N, B, R, Q, K, p, n, b, r, q, k
};
// ASCII pieces
char ascii_pieces[12] = "PNBRQKpnbrqk";

// unicode pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

// convert ASCII character pieces to encoded constants
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

const char *coordinates[] = {
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

static inline void set_bit(uint64_t *bitboard, int square) {
    *bitboard |= (1ULL << square);
}

static inline int get_bit(uint64_t bitboard, int square) {
    return (bitboard >> square) & 1ULL;
}

static inline void pop_bit(uint64_t *bitboard, int square) {
    *bitboard &= ~(1ULL << square);
}

uint64_t piece_bitboards[12];
uint64_t occupancy_bitboards[3];
int size_to_move = -1;
int enpassant = no_sq;
int castle;

static inline int count_bits(uint64_t bitboard)
{
  int count = 0;
  while(bitboard)
  {
    count++;
    bitboard &= bitboard - 1;
  }
  return count;
}

static inline int get_ls1b_index(uint64_t bitboard)
{
  if (bitboard)
  {
    return count_bits((bitboard & -bitboard) - 1);
  }
  else
  {
    return -1;
  }
}

void print_bitboard(uint64_t bitboard)
{
  printf("\n");
  for (int rank = 0; rank < 8; rank++)
  {
    for (int file = 0; file < 8; file++)
    {
      int square = rank * 8 + file;
      if (!file)
      {
        printf("%d  ", 8 - rank);
      }
      printf(" %d", get_bit(bitboard, square) ? 1 : 0);
    }
    printf("\n");
  }
  printf("\n    a b c d e f g h\n\n");
  printf("Bitboard: %" PRIu64 "\n", bitboard);
}

const uint64_t not_file_a = 18374403900871474942ULL;
const uint64_t not_file_h = 9187201950435737471ULL;
const uint64_t not_file_hg = 4557430888798830399ULL;
const uint64_t not_file_ab = 18229723555195321596ULL;

const int bishop_relevant_occupancy_bits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

const int rook_relevant_occupancy_bits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

// rook magic numbers
uint64_t rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

// bishop magic numbers
uint64_t bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

uint64_t pawn_attacks[2][64];
uint64_t knight_attacks[64];
uint64_t king_attacks[64];
uint64_t bishop_masks[64];
uint64_t rook_masks[64];
uint64_t bishop_attacks[64][512];
uint64_t rook_attacks[64][4096];

uint64_t mask_pawn_attacks(int side, int square)
{
  uint64_t attacks_bitboard = 0;
  uint64_t piece_bitboard = 0;
  set_bit(&piece_bitboard, square);
  if (!side)
  {
    if ((piece_bitboard >> 7) & not_file_a) attacks_bitboard |= (piece_bitboard >> 7);
    if ((piece_bitboard >> 9) & not_file_h) attacks_bitboard |= (piece_bitboard >> 9);
  }
  else
  {
    if ((piece_bitboard << 7) & not_file_h) attacks_bitboard |= (piece_bitboard << 7);
    if ((piece_bitboard << 9) & not_file_a) attacks_bitboard |= (piece_bitboard << 9);
   
  } 
  return attacks_bitboard;
}

uint64_t mask_knight_attacks(int square)
{
  uint64_t attacks_bitboard = 0;
  uint64_t piece_bitboard = 0;
  set_bit(&piece_bitboard, square);

  if ((piece_bitboard >> 17) & not_file_h) attacks_bitboard |= (piece_bitboard >> 17);
  if ((piece_bitboard >> 15) & not_file_a) attacks_bitboard |= (piece_bitboard >> 15);
  if ((piece_bitboard >> 10) & not_file_hg) attacks_bitboard |= (piece_bitboard >> 10);
  if ((piece_bitboard >> 6) & not_file_ab) attacks_bitboard |= (piece_bitboard >> 6);
  if ((piece_bitboard << 17) & not_file_a) attacks_bitboard |= (piece_bitboard << 17);
  if ((piece_bitboard << 15) & not_file_h) attacks_bitboard |= (piece_bitboard << 15);
  if ((piece_bitboard << 10) & not_file_ab) attacks_bitboard |= (piece_bitboard << 10);
  if ((piece_bitboard << 6) & not_file_hg) attacks_bitboard |= (piece_bitboard << 6);
 
    
  return attacks_bitboard;
}


uint64_t mask_king_attacks(int square)
{
  uint64_t attacks_bitboard = 0;
  uint64_t piece_bitboard = 0;
  set_bit(&piece_bitboard, square);

  if ((piece_bitboard >> 7) & not_file_a) attacks_bitboard |= (piece_bitboard >> 7);
  if (piece_bitboard >> 8) attacks_bitboard |= (piece_bitboard >> 8);
  if ((piece_bitboard >> 9) & not_file_h) attacks_bitboard |= (piece_bitboard >> 9);
  if ((piece_bitboard >> 1) & not_file_h) attacks_bitboard |= (piece_bitboard >> 1);
  if ((piece_bitboard << 1) & not_file_a) attacks_bitboard |= (piece_bitboard << 1);
  if ((piece_bitboard << 9) & not_file_a) attacks_bitboard |= (piece_bitboard << 9);
  if (piece_bitboard << 8) attacks_bitboard |= (piece_bitboard << 8);
  if ((piece_bitboard << 7) & not_file_h) attacks_bitboard |= (piece_bitboard << 7);
 
    
  return attacks_bitboard;
}

uint64_t mask_bishop_attacks(int square)
{
  uint64_t attacks_bitboard = 0;

  int rank, file;
  int target_rank = square / 8;
  int target_file = square % 8;

  for (rank = target_rank + 1, file = target_file + 1; rank <= 6 && file <= 6; rank++, file++) attacks_bitboard |= (1ULL << (rank * 8 + file));
  for (rank = target_rank - 1, file = target_file + 1; rank >= 1 && file <= 6; rank--, file++) attacks_bitboard |= (1ULL << (rank * 8 + file));
  for (rank = target_rank + 1, file = target_file - 1; rank <= 6 && file >= 1; rank++, file--) attacks_bitboard |= (1ULL << (rank * 8 + file));
  for (rank = target_rank - 1, file = target_file - 1; rank >= 1 && file >= 1; rank--, file--) attacks_bitboard |= (1ULL << (rank * 8 + file));

  return attacks_bitboard;
}

uint64_t mask_rook_attacks(int square)
{
  uint64_t attacks_bitboard = 0;

  int rank, file;
  int target_rank = square / 8;
  int target_file = square % 8;

  for (rank = target_rank + 1; rank <= 6; rank++) attacks_bitboard |= (1ULL << (rank * 8 + target_file));    
  for (rank = target_rank - 1; rank >= 1; rank--) attacks_bitboard |= (1ULL << (rank * 8 + target_file));
  for (file = target_file + 1; file <= 6; file++) attacks_bitboard |= (1ULL << (target_rank * 8 + file));
  for (file = target_file - 1; file >= 1; file--) attacks_bitboard |= (1ULL << (target_rank * 8 + file));
      
  return attacks_bitboard;
}


uint64_t bishop_attacks_with_blocks(int square, uint64_t occupancy)
{
  uint64_t attacks_bitboard = 0;

  int rank, file;
  int target_rank = square / 8;
  int target_file = square % 8;

  for (rank = target_rank + 1, file = target_file + 1; rank <= 7 && file <= 7; rank++, file++)
  {
    attacks_bitboard |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy) break;
  }
  for (rank = target_rank - 1, file = target_file + 1; rank >= 0 && file <= 7; rank--, file++)
  {
    attacks_bitboard |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy) break;
  }
  for (rank = target_rank + 1, file = target_file - 1; rank <= 7 && file >= 0; rank++, file--)
  {
    attacks_bitboard |= (1ULL << (rank * 8 + file)); 
    if ((1ULL << (rank * 8 + file)) & occupancy) break;
  }
  for (rank = target_rank - 1, file = target_file - 1; rank >= 0 && file >= 0; rank--, file--)
  {
    attacks_bitboard |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy) break;
  }      
  return attacks_bitboard;
}


uint64_t rook_attacks_with_blocks(int square, uint64_t occupancy)
{
  uint64_t attacks_bitboard = 0;

  int rank, file;
  int target_rank = square / 8;
  int target_file = square % 8;

  for (rank = target_rank + 1; rank <= 7; rank++)
  {
    attacks_bitboard |= (1ULL << (rank * 8 + target_file));
    if ((1ULL << (rank * 8 + target_file)) & occupancy) break;
  }
      
  for (rank = target_rank - 1; rank >= 0; rank--) 
  {
    attacks_bitboard |= (1ULL << (rank * 8 + target_file));
    if ((1ULL << (rank * 8 + target_file)) & occupancy) break;
  }
      
  for (file = target_file + 1; file <= 7; file++)
  {
    attacks_bitboard |= (1ULL << (target_rank * 8 + file));
    if ((1ULL << (target_rank * 8 + file)) & occupancy) break;
  }
  
  for (file = target_file - 1; file >= 0; file--)
  {
    attacks_bitboard |= (1ULL << (target_rank * 8 + file));
    if ((1ULL << (target_rank * 8 + file)) & occupancy) break;
  }
      
  return attacks_bitboard;
}

uint64_t set_occupancy(int index, int bits_in_mask, uint64_t attack_mask)
{
  uint64_t occupancy = 0ULL;

  for (int count = 0; count < bits_in_mask; count++)
  {
    int square = get_ls1b_index(attack_mask);
    
    pop_bit(&attack_mask, square);

    if (index & (1 << count)) occupancy |= (1ULL << square);
  }
  
  return occupancy;
}

uint32_t random_state = 1804289383;

uint32_t get_random_u32_number()
{
  uint32_t number = random_state;

  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  random_state = number;

  return number;
}

uint64_t get_random_u64_number()
{
  uint64_t n1, n2, n3, n4;

  n1 = (uint64_t)(get_random_u32_number()) & 0xFFFF;
  n2 = (uint64_t)(get_random_u32_number()) & 0xFFFF;
  n3 = (uint64_t)(get_random_u32_number()) & 0xFFFF;
  n4 = (uint64_t)(get_random_u32_number()) & 0xFFFF;

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

uint64_t generate_magic_number()
{
  return get_random_u64_number() & get_random_u64_number() & get_random_u64_number();  
}

uint64_t find_magic_number(int square, int relevant_bits, int bishop)
{
  uint64_t occupancy[4096];
  uint64_t attacks[4096];
  uint64_t used_attacks[4096];
  uint64_t attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

  int occupancy_indices = 1 << relevant_bits;

  for (int index = 0; index < occupancy_indices; index++)
  {
    occupancy[index] = set_occupancy(index, relevant_bits, attack_mask);

    attacks[index] = bishop ? bishop_attacks_with_blocks(square, occupancy[index]) : rook_attacks_with_blocks(square, occupancy[index]);
  }

  for (int random_count = 0; random_count < 100000000; random_count++)
  {
    uint64_t magic_number = generate_magic_number();

    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

    memset(used_attacks, 0ULL, sizeof(used_attacks));
    int index, fail;

    for (index = 0, fail = 0; !fail && index < occupancy_indices; index++)
    {
      int magic_index = (int)((occupancy[index] * magic_number) >> (64 - relevant_bits));

      if (used_attacks[magic_index] == 0ULL) used_attacks[magic_index] = attacks[index];
      else if (used_attacks[magic_index] != attacks[index]) fail = 1;
    }
    if (!fail) return magic_number;
  }
  printf("   Magic Number Failed!");
  return 0ULL;
}

void init_magic_numbers()
{
  for (int square = 0; square < 64; square++)
  {
    rook_magic_numbers[square] = find_magic_number(square, rook_relevant_occupancy_bits[square], rook);
  }
  for (int square = 0; square < 64; square++)
  {    
    bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_occupancy_bits[square], bishop);
  }
}

void init_leaper_attacks()
{
  for (int square = 0; square < 64; square++)
  {
    pawn_attacks[white][square] = mask_pawn_attacks(white, square);
    pawn_attacks[black][square] = mask_pawn_attacks(black, square);

    knight_attacks[square] = mask_knight_attacks(square);

    king_attacks[square] = mask_king_attacks(square);
  }
}

void init_slider_attacks(int bishop)
{
  for (int square = 0; square < 64; square++)
  {
    bishop_masks[square] = mask_bishop_attacks(square);
    rook_masks[square] = mask_rook_attacks(square);

    uint64_t attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

    int relevant_bit_count = count_bits(attack_mask);
    int occupancy_indices = (1 << relevant_bit_count);

    for (int index = 0; index < occupancy_indices; index++)
    {
      if (bishop)
      {
        uint64_t occupancy = set_occupancy(index, relevant_bit_count, attack_mask);
        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_occupancy_bits[square]);
        bishop_attacks[square][magic_index] = bishop_attacks_with_blocks(square, occupancy);
      }
      else
      {
        uint64_t occupancy = set_occupancy(index, relevant_bit_count, attack_mask);
        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_occupancy_bits[square]);
        rook_attacks[square][magic_index] = rook_attacks_with_blocks(square, occupancy);
      }
    }
  }
}

// get attacks assuming current board occupancy
static inline uint64_t get_bishop_attacks(int square, uint64_t occupancy)
{
  occupancy &= bishop_masks[square];
  occupancy *= bishop_magic_numbers[square];
  occupancy >>= 64 - bishop_relevant_occupancy_bits[square];

  return bishop_attacks[square][occupancy];
}

static inline uint64_t get_rook_attacks(int square, uint64_t occupancy)
{
  occupancy &= rook_masks[square];
  occupancy *= rook_magic_numbers[square];
  occupancy >>= 64 - rook_relevant_occupancy_bits[square];

  return rook_attacks[square][occupancy];
}

void init_all()
{
  init_leaper_attacks();
  init_slider_attacks(bishop);
  init_slider_attacks(rook);
}

int main()
{
  init_all();

  uint64_t occupancy = 0;

  set_bit(&occupancy, c5);

  print_bitboard(occupancy);

  print_bitboard(get_bishop_attacks(d4, occupancy));

  return 0;
}
