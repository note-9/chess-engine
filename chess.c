#include <stdio.h>
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
  a1, b1, c1, d1, e1, f1, g1, h1
};

enum {
  white,
  black
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

uint64_t pawn_attacks[2][64];
uint64_t knight_attacks[64];
uint64_t king_attacks[64];
uint64_t bishop_attacks[64];
uint64_t rook_attacks[64];

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

  for (rank = target_rank + 1, file = target_file + 1; rank <= 7 && file <= 7; rank++, file++) attacks_bitboard |= (1ULL << (rank * 8 + file));
  for (rank = target_rank - 1, file = target_file + 1; rank >= 0 && file <= 7; rank--, file++) attacks_bitboard |= (1ULL << (rank * 8 + file));
  for (rank = target_rank + 1, file = target_file - 1; rank <= 7 && file >= 0; rank++, file--) attacks_bitboard |= (1ULL << (rank * 8 + file));
  for (rank = target_rank - 1, file = target_file - 1; rank >= 0 && file >= 0; rank--, file--) attacks_bitboard |= (1ULL << (rank * 8 + file));

  return attacks_bitboard;
}

uint64_t mask_rook_attacks(int square)
{
  uint64_t attacks_bitboard = 0;

  int rank, file;
  int target_rank = square / 8;
  int target_file = square % 8;

  for (rank = target_rank + 1; rank <= 7; rank++) attacks_bitboard |= (1ULL << (rank * 8 + target_file));    
  for (rank = target_rank - 1; rank >= 0; rank--) attacks_bitboard |= (1ULL << (rank * 8 + target_file));
  for (file = target_file + 1; file <= 7; file++) attacks_bitboard |= (1ULL << (target_rank * 8 + file));
  for (file = target_file - 1; file >= 0; file--) attacks_bitboard |= (1ULL << (target_rank * 8 + file));
      
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

void init_slider_attacks(uint64_t occupancy)
{
  for (int square = 0; square < 64; square++)
  {
    bishop_attacks[square] = mask_bishop_attacks(square);
    rook_attacks[square] = mask_rook_attacks(square);
  }
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

int main()
{
  init_leaper_attacks();
  uint64_t occupancy = 0ULL;;
  printf("CHESS ENGINE\n");
 /* for (int square = 0; square < 64; square++)
  {
    print_bitboard(pawn_attacks[black][square]);
  }
  */
/*  for (int square = 0; square < 64; square++)
  {
    print_bitboard(knight_attacks[square]);
  }
*/
 
/*  for (int square = 0; square < 64; square++)
  {
    print_bitboard(king_attacks[square]);
  }
*/

/*  for (int square = 0; square < 64; square++)
  {
    print_bitboard(bishop_attacks[square]);
  }
*/
  for (int square = 0; square < 64; square++)
  {
    print_bitboard(mask_rook_attacks(square, occupancy));
  } 
  return 0;
}
