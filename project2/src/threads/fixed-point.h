

#define P 17
#define Q 14
#define F 1 << (Q) 


#define CONVERT_INT_TO_FP(n) (n) * (F)
#define CONVERT_FP_TO_INT_ZERO(x) (x)/(F)
#define CONVERT_FP_TO_INT_NEAR(x) ((x) >= 0 ? ((x)+(F)/2)/(F) : ((x)-(F)/2)/(F)) 
#define ADD_INT(x, n) (x) + (n) * (F)
#define SUB_INT(x, n) (x) - (n) * (F)
#define MUL(x, y) ((int64_t)(x)) * (y) / (F)
#define DIV(x, y) ((int64_t)(x)) * (F) / (y)

