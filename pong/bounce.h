// Isaac Fowler
// bounce.h

/* some settings for the game   */

#define BLANK           ' '
#define DFL_SYMBOL      'o'
#define TOP_ROW         5
#define BOT_ROW         20
#define LEFT_EDGE       10
#define RIGHT_EDGE      70
#define X_INIT          10              /* starting col         */
#define Y_INIT          10              /* starting row         */
#define TICKS_PER_SEC   50              /* affects speed        */


/* the ping pong ball */

//Ping Pong Ball
struct ppball {
                int     y_pos, x_pos,
                        y_ttm, x_ttm, //ticks to move between each redraw
                        y_ttg, x_ttg, //ticks to go before next redraw
                        y_dir, x_dir;
                char    symbol ;

};

//Ping Pong Paddle
struct pppaddle {
        int pad_top, pad_bot, pad_col;
        char padchar;
};

