#ifdef		SCREEN_BUF_INIT
void		SCREEN_BUF_INIT(void)
{
    unsigned int i, j;
    TYPE *p = (TYPE *) SCREEN_TOP;

    for (j = SCREEN_HEIGHT; j; j--) {
	for (i = SCREEN_WIDTH; i; i--) {
	    *p++ = BLACK;
	}
    }
}
#endif		/* SCREEN_BUF_INIT */
