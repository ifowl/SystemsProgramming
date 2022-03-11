// Isaac Fowler
// Pong.c

#include        <curses.h>
#include        <signal.h>
#include        "bounce.h"
#include        <sys/time.h>

struct ppball the_ball;
struct pppaddle paddle;
int balls_left = 3;

//////////////////////
/* Paddle functions */
//////////////////////

void paddle_init() {
        paddle.padchar = '#';
        paddle.pad_top = TOP_ROW + 2;
        paddle.pad_bot = TOP_ROW + 8;
        paddle.pad_col = RIGHT_EDGE+1;
        for (int i = paddle.pad_top; i < paddle.pad_bot; ++i) {
                mvaddch(i, paddle.pad_col, paddle.padchar);
        }
        refresh();
}

void paddle_up() {
        if (paddle.pad_top > TOP_ROW) {
                paddle.pad_top -= 1;
                paddle.pad_bot -= 1;
                mvaddch(paddle.pad_bot, paddle.pad_col, BLANK);
                for (int i = paddle.pad_top; i < paddle.pad_bot; ++i) {
                        mvaddch(i, paddle.pad_col, paddle.padchar);
                }
        }
        refresh();
}

void paddle_down() {
        if (paddle.pad_bot < BOT_ROW+1) {
                mvaddch(paddle.pad_top, paddle.pad_col, BLANK);
                paddle.pad_top += 1;
                paddle.pad_bot += 1;
                for (int i = paddle.pad_top; i < paddle.pad_bot; ++i) {
                        mvaddch(i, paddle.pad_col, paddle.padchar);
                }
        }
        refresh();
}


int paddle_contact(int y, int x) {
        if (y >= paddle.pad_top && y <= paddle.pad_bot)
                return 1;
        else return 0;
}

////////////////
/* Main Logic */
////////////////

void set_up();
void wrap_up();

set_ticker( n_msecs )
{
        struct itimerval new_timeset;
        long    n_sec, n_usecs;

        n_sec = n_msecs / 1000 ;
        n_usecs = ( n_msecs % 1000 ) * 1000L ;

        new_timeset.it_interval.tv_sec  = n_sec;        /* set reload  */
        new_timeset.it_interval.tv_usec = n_usecs;      /* new ticker value */
        new_timeset.it_value.tv_sec     = n_sec  ;      /* store this   */
        new_timeset.it_value.tv_usec    = n_usecs ;     /* and this     */

        return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

int main()
{
        int     c;
        srand(getpid());

        set_up();
        serve();

        while ( balls_left > 0 && (c = getchar()) != 'Q') {
                if (c == 'j') paddle_down();
                else if (c == 'k') paddle_up();
        }

        wrap_up();
}

void wrap_up()
{

        set_ticker( 0 );
        endwin();               /* put back to normal   */
}

/////////////
/* Visuals */
/////////////

void score() {
        move(BOT_ROW+3,10);
        printw("Balls Remaning: %d", balls_left);
        refresh();
}


void drawWalls() {
        clear();
        for (int i = TOP_ROW; i < BOT_ROW+1; ++i) {
                move(i, LEFT_EDGE-1);
                addstr("|");
        }
        for (int i = LEFT_EDGE-1; i < RIGHT_EDGE+2; ++i) {
                move(TOP_ROW-1, i);
                addstr("-");
                move(BOT_ROW+1, i);
                addstr("-");
        }
        refresh();
}
void set_up(int balls_left)
{

        initscr();
        noecho();
        crmode();

        drawWalls();
        paddle_init();
        score();

}

///////////////////
/* Ball movement */
///////////////////

void serve() {
        void ball_move(int);

        int yDirection = (rand() % 2);
        if (yDirection == 0)
                yDirection = -1;

        int x_ttm = (rand() % 5) + 3;
        int y_ttm = (rand() % 5) + 3;

        the_ball.y_pos = Y_INIT;
        the_ball.x_pos = X_INIT;
        the_ball.y_ttg = the_ball.y_ttm = y_ttm ;
        the_ball.x_ttg = the_ball.x_ttm = x_ttm ;
        the_ball.y_dir = yDirection  ; //Can be either 1 or -1 to indicate serving up or down.
        the_ball.x_dir = 1  ;
        the_ball.symbol = DFL_SYMBOL ;

        signal( SIGINT , SIG_IGN );
        mvaddch( the_ball.y_pos, the_ball.x_pos, the_ball.symbol  );
        refresh();

        signal( SIGALRM, ball_move );
        set_ticker( 1000 / TICKS_PER_SEC );     /* send millisecs per tick */
}

void ball_move(int signum)
{
        int     y_cur, x_cur, moved;

        signal( SIGALRM , SIG_IGN );            /* dont get caught now  */
        y_cur = the_ball.y_pos ;                /* old spot             */
        x_cur = the_ball.x_pos ;
        moved = 0 ;

        if ( the_ball.y_ttm > 0 && the_ball.y_ttg-- == 1 ){
                the_ball.y_pos += the_ball.y_dir ;      /* move */
                the_ball.y_ttg = the_ball.y_ttm  ;      /* reset*/
                moved = 1;
        }

        if ( the_ball.x_ttm > 0 && the_ball.x_ttg-- == 1 ){
                the_ball.x_pos += the_ball.x_dir ;      /* move */
                the_ball.x_ttg = the_ball.x_ttm  ;      /* reset*/
                moved = 1;
        }

        if ( moved ){
                mvaddch( y_cur, x_cur, BLANK );
                mvaddch( y_cur, x_cur, BLANK );
                mvaddch( the_ball.y_pos, the_ball.x_pos, the_ball.symbol );
                if (bounce_or_lose( &the_ball ) == -1) {
                        serve();
                }
                move(LINES-1,COLS-1);
                refresh();
        }
        signal( SIGALRM, ball_move);            /* for unreliable systems */

}

int bounce_or_lose(struct ppball *bp)
{
        int     return_val = 0 ;

        if ( bp->y_pos == TOP_ROW ){
                bp->y_dir = 1 ;
                return_val = 1 ;
        } else if ( bp->y_pos == BOT_ROW ){
                bp->y_dir = -1 ;
                return_val = 1;
        }
        if ( bp->x_pos == LEFT_EDGE ){
                bp->x_dir = 1 ;
                return_val = 1 ;
        } else if ( bp->x_pos == RIGHT_EDGE ){
                if (paddle_contact(bp->y_pos, bp->x_pos) == 1) { //paddle is there so it hit
                        bp->x_dir = -1;
                        return_val = 1;
                        bp->x_ttm = (rand() % 5) + 3; //randomize x-speed after hit
                } else { //paddle isn't there so it missed
                        return_val = -1;
                        mvaddch(bp->y_pos, bp->x_pos, BLANK);
                        --balls_left;
                        score();
                }
        }

        return return_val;
}
