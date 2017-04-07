
/************************************* INCLUDES *************************************/

/************************************** MACROS **************************************/

#define assert( a ) if (!a) { __assert(__FUNCTION__, __FILE__, __LINE__,  #a); }
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
#define UNUSED(x) (void)(x)

#define RGB_R_PIN      3U
#define RGB_G_PIN      4U
#define RGB_B_PIN      5U

/*********************************** DEFINITIONS  ************************************/

void printConfigMenu(void);
uint32_t serialReadBytes(uint8_t *p_buff, uint32_t len, uint32_t timeout);
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp);
void error(void);