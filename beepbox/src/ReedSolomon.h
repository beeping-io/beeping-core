#ifndef __REEDSOLOMON__
#define __REEDSOLOMON__

#include <vector>

namespace BEEPING
{
  class ReedSolomon
  {
  public:
    ReedSolomon();
    ~ReedSolomon();

    void GenerateGaloisField();
    void GeneratePoly();
    void Encode();
    void Decode();

    int mm;          /* RS code over GF(2**4) - change to suit */
    int nn;          /* nn=2**mm -1   length of codeword */
    int tt;          /* number of errors that can be corrected */
    int kk;          /* kk = nn-2*tt  */
    
    int msg_len; /* for shortened RS code */

    //for encode
    int *pp; /* specify irreducible polynomial coeffts */
    int *alpha_to;
    int *index_of;
    int *gg;

    int *recd;
    int *data;
    int *bb;

    //For decode
    int **elp, *d, *l, *u_lu, *s;
    int *root, *loc, *z, *err, *reg;


    // Voctro API
    // encoding
    void SetMessage(const std::vector<int> message);
    void GetCode(std::vector<int> &code);
    // decoding
    void SetCode(const std::vector<int> code);
    void GetMessage(std::vector<int> &message);

  };
}

#endif //__REEDSOLOMON__
