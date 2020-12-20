#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define MAP_X 3
#define MAP_Y 2
#define MAP_WIDTH 50
#define MAP_HEIGHT 20

void draw_map(); //게임화면 틀을 생성하는 함수
void start();     //게임 맵과 뱀을 로드하여 게임 환경을 설정하는 함수
void title(void);//게임 시작화면을 로드하는 함수
void create_food();//뱀의 먹이를 생성하는 함수
int eat();  //뱀이 먹이를 먹었는지 확인하는 함수
int set_ticker(int n_msecs);//타이머 설정 함수
void move_msg(int signum);//SIGALRM 시그널 처리 함수
void game_over();//게임오버 되었을 때 출력해주는 함수
void create_poison();//뱀의 독을 생성하는 함수
int eat_poison();//뱀이 독을 먹었는지 확인하는 함수

int score=0; //current score
int dirX=1;     //X축 방향을 나타내는 변수
int dirY=0;     //Y축 방향을 나타내는 변수
int length=0;       //뱀 길이를 나타내는 변수
int food[2];        //먹이의 좌표를 나타내는 변수
int poison[2];      //독의 좌표를 나타내는 변수
int check=0;        //먹이 생성함수를 실행하였는지를 체크하는 변수 
int x[100], y[100];     //뱀 각각의 머리 몸통 꼬리의 위치를 차례대로 담는 배열

int main(){
    int ndelay=0;
    int delay=200;
    int c;
    char str_score[10];
     
    initscr();      //turn on curses    
    noecho();       
    clear();        //clear screen
    title();        //게임 시작화면 로드
    
    //space키가 입력되면 게임 맵과 뱀이 로드되고 게임을 시작한다.
    while(1){
        c = getch(); //키 입력을 받는 함수
        if(c==' ') start(); break;
    }
    
    signal(SIGALRM, move_msg); //SIGALRM 시그널 핸들러로 move_msg 함수를 사용한다. delay가 200이므로 SIGALRM 신호가 0.2초 단위로 계속 발생한다.
    set_ticker(delay); //타이머 설정 함수
    
    while(1){
        ndelay=0;
        c=getch();
        if(c=='Q' || c=='q') //키보드로 Q나 q가 입력될 경우 게임을 중단한다.
            break;
        else if(c=='B') { //키보드로 ↓가 입력될 경우 dirY를 1로 설정하여 Y축 방향을 아래로 설정한다.
            dirY=1; dirX=0;
        }
        else if(c=='A') { //키보드로 ↑가 입력될 경우 dirY를 -1로 설정하여 Y축 방향을 위로 설정한다.
            dirY=-1; dirX=0;
        }
        else if(c=='D') { //키보드로 ←가 입력될 경우 dirX를 -1로 설정하여 X축 방향을 왼쪽으로 설정한다.
            dirX=-1; dirY=0;
        }
        else if(c=='C') { //키보드로 →가 입력될 경우 dirX를 1로 설정하여 X축 방향을 오른쪽으로 설정한다.
            dirX=1; dirY=0;
        }
        else if(c=='f' && delay>2) ndelay=delay/2; //키보드로 f가 입력될 경우 delay를 2로 나누어 속도를 2배 향상시킨다.
        else if(c=='s') ndelay=delay*2; // 키보드로 s가 입력될 경우 delay에 2를 곱해주어 속도를 2배 느리도록 설정한다.
        
        if(check==0 && score==40){  //독 생성 함수를 한번도 실행한 적이 없으며 score 40점을 달성할 경우, 독을 생성하는 함수를 실행한다.
             check=1;                  //creat_poison() 함수를 실행하였다면 check에 1을 저장하여 독을 생성하였음을 check한다. 독 생성 함수를 한번만 실행하면 다시 실행해줄 필요가 없기 때문에 체크한다.
             create_poison();           //독 생성 함수를 실행한다
        }
        
        if(ndelay>0) set_ticker(delay=ndelay);  
    }
    
    endwin();                   //reset the tty etc
}


// 경계선(게임 틀)을 그리는 함수  <curses.h>라이브러리를 사용하여 내장함수 move()와 addstr()로 게임 틀을 그려준다.
void draw_map(){ 
    int i,j;
    
    for(i=0; i<MAP_WIDTH; i++){
        move(MAP_Y,MAP_X+i);     
        addstr("*");
    }
    
    for(i=MAP_Y+1; i<MAP_Y+MAP_HEIGHT-1; i++){
        move(i, MAP_X);
        addstr("*");
        move(i,MAP_X+MAP_WIDTH-1);
        addstr("*");
        
    }
    
    for(i=0; i<MAP_WIDTH; i++){
        move(MAP_Y+MAP_HEIGHT-1, MAP_X+i);
        addstr("*");
    }

}

//게임 시작화면 로드하는 함수
void title(void){
    draw_map();
    
    move(MAP_Y+5, MAP_X+(MAP_WIDTH/2)-13);
    addstr("+--------------------------+");
    move(MAP_Y+6, MAP_X+(MAP_WIDTH/2)-13);
    addstr("|        S N A K E         |");
    move(MAP_Y+7, MAP_X+(MAP_WIDTH/2)-13);
    addstr("+--------------------------+");
    
    move(MAP_Y+9, MAP_X+(MAP_WIDTH/2)-12);
    addstr("< PRESS ANY KEY TO START >");
    
    move(MAP_Y+11, MAP_X+(MAP_WIDTH/2)-17);
    addstr("- up, down, right, left key : Move");
    move(MAP_Y+12, MAP_X+(MAP_WIDTH/2)-17);
    addstr("- s, f key : Speed");
    move(MAP_Y+13, MAP_X+(MAP_WIDTH/2)-17);
    addstr("- Q key : Quit");
    
    refresh();
}

//게임 맵과 뱀을 로드하여 게임 환경을 설정하는 함수
void start(){
    char buf[100];
    int i;
    clear();
    draw_map();             //경계선(게임 틀)을 그리는 함수
    
    move(MAP_HEIGHT+2, MAP_X+1);        //커서 이동
    addstr("score : ");                 //해당 위치에 점수 정보 출력
    
    sprintf(buf, "%d", score);      //정수는 addstr로 출력할 수 없으므로 buf에 문자열로 저장한 후 addstr로 화면 출력
    addstr(buf);

    length=1;                       
    
    for(i=0;i<length; i++){
        x[i]=MAP_WIDTH/2+i;
        y[i]=MAP_HEIGHT/2;
    }
    move(MAP_Y+y[0], MAP_X+x[0]);       //뱀 머리 출력 위치로 커서 이동
    addstr("@");                        //해당 위치에 뱀 머리 출력
    
    create_food();                      //먹이 생성 함수 호출
    
    refresh();
}

//먹이를 생성하는 함수
void create_food(){
    do{
        srand(time(NULL));
        food[0]=rand()%(MAP_HEIGHT-2)+3;                //food[0] 배열에 랜덤숫자 배정하여 먹이의 y위치 랜덤 지정(food[0]는 먹이의 y축 위치 의미)
        srand(time(NULL));
        food[1]=rand()%(MAP_WIDTH-1)+4;                 //food[1] 배열에 랜덤숫자 배정하여 먹이의 x위치 랜덤 지정(food[1]는 먹이의 x축 위치 의미)
    }while(eat());                                      //뱀이 먹이를 먹었을 경우에 계속 실시

    move(food[0],food[1]);                              //해당 위치로 커서 이동
    addstr("O");                                        //해당 커서 위치로 먹이 출력
    
    refresh();                                          //update the screen
}

//뱀이 먹이를 먹었는지 확인하는 함수
int eat(){
    int touch=0;
    
    if(x[0]+MAP_X==food[1] && y[0]+MAP_Y==food[0]){             
        move(food[0],food[1]);                                  //먹이가 존재하던 부분을 지워주기 위해 그 위치로 이동하여 빈칸 출력
        addstr(" ");                                            
        touch=1;                                                //뱀이 먹이를 먹었을 경우 touch를 1로 지정하고 그 값을 반환
    }
    
    refresh();                                                  //update the screen

    return touch;
}

//타이머를 설정해주는 함수
int set_ticker( int n_msecs )
{
    struct itimerval new_timeset;
    long    n_sec, n_usecs;
    n_sec = n_msecs / 1000 ;        /* int part*/
    n_usecs = ( n_msecs % 1000 ) * 1000L ;  /* remainder*/

    new_timeset.it_interval.tv_sec = n_sec;        /* set reload */
    new_timeset.it_interval.tv_usec = n_usecs;  /* new ticker value */
    new_timeset.it_value.tv_sec     = n_sec;    /* store this */
    new_timeset.it_value.tv_usec    = n_usecs;    /* and this */

    /*정할 타이머 종류는 ITIMER_REAL,(타이머가 ITIMER_REAL일 경우, SIGALRM을 보낸다.) 
    설정할 타이머 정보를 저장한 구조체 포인터 new_timeset, 이전 타이머 정보를 저장할 구조체 포인터는 NULL로 함수 인자를 설정하여 타이머를 set하고 return한다.*/
    return setitimer(ITIMER_REAL, &new_timeset, NULL); 
}

//SIGALRM 시그널을 처리해주는 함수
void move_msg(int signum){
    int i;
    char buf[100];

    signal(SIGALRM, move_msg);              //SIGALRM 발생시 signal 핸들러로 move_msg함수 호출

    if(eat()){                              //만약 먹이를 먹었을 경우
        score+=10;                          //점수 10점 적립
        move(MAP_HEIGHT+2, MAP_X+1);           
        addstr("score : ");                     //점수판 출력
        sprintf(buf, "%d", score);              //score는 정수이므로 addstr로 출력할 수 없기 때문에 buf에 문자열로 저장한후 addstr로 출력
        addstr(buf);
        refresh();                              //update the screen
        create_food();                          //먹이 생성 함수 호출
        length++;                               
        x[length-1]=x[length-2];                //뱀 꼬리 한개 만들어주기
        y[length-1]=y[length-2];                
    }
    
    if(score>=40){                              //점수가 40이상일시 독 만들어주기
        if(eat_poison()){                        //만약 독을 먹었을 경우      
            move(y[length-1]+MAP_Y,x[length-1]+MAP_X);   //뱀의 꼬리 부분으로 가기
            addstr(" ");                                 //꼬리 없애주기
            --length;                                    //뱀 길이 하나 줄이기
            if(length==0){                               //만약 뱀의 길이가 0이면 게임 종료 
                game_over();
                return;
            }
            create_poison();                     //다시 독 만들어주기
        }
    }

    if(x[0]==0 || x[0]==MAP_WIDTH-1 ||y[0]==0 || y[0]==MAP_HEIGHT-1){   //만약 뱀이 화면 벽에 다으면 게임 종료
        game_over();
        return;
    }

    if(length>=5){                   //만약 뱀이 자기 몸이랑 접촉하면 게임 종료
        for(i=1; i<length; i++){
            if(x[0]==x[i] && y[0]==y[i]){
                game_over();
                return;
            }
        }
    }    
    
    
    /*사용자가 입력한 방향으로 움직이는 과정*/
   //뱀의 마지막 꼬리가 그려진 부분을 없애주고 하나씩 당겨주기
    move(MAP_Y+y[length-1], MAP_X+x[length-1]);  
    addstr(" ");
    
    //뱀의 마지막 꼬리가 그려진 부분을 없애주고 하나씩 당겨주기
    for(i=length-1; i>0; i--){
        x[i]=x[i-1];
        y[i]=y[i-1];
    }
    
    //뱀의 꼬리 부분 그려주기
    for(i=1; i<length; i++){
        move(y[i]+MAP_Y,x[i]+MAP_X);
        addstr("Z");
    }
    //뱀의 머리 위치를 direction 변수로 조정해주고 screen 창에 그려주기
    if(length>0){
        y[0]=y[0]+dirY;
        x[0]=x[0]+dirX;
        move(MAP_Y+y[0], MAP_X+x[0]);
        addstr("@");
    }
    
    refresh();
}


//게임오버 되었을 때 실행되는 함수 :게임 오버 화면을 만들어준다.
void game_over(){
    clear();
    draw_map();
    
    move(MAP_Y+5, MAP_X+(MAP_WIDTH/2)-13);
    addstr("+----------------------+");
    move(MAP_Y+6, MAP_X+(MAP_WIDTH/2)-13);
    addstr("|      GAME OVER..     |");
    move(MAP_Y+7, MAP_X+(MAP_WIDTH/2)-13);
    addstr("+----------------------+");

    move(MAP_Y+9, MAP_X+(MAP_WIDTH/2)-12);
    addstr("Your score :  ");

    char buf[100];

    sprintf(buf, "%d", score);
    addstr(buf);    
}

//독을 생성하는 함수
void create_poison(){
    do{
        srand(time(NULL));                    //게임 화면 내의 범위에서 무작위로 독 뿌려주기
        poison[0]=rand()%(MAP_HEIGHT-2)+3;      //독의 y좌표 랜덤 지정
        srand(time(NULL));
        poison[1]=rand()%(MAP_WIDTH-1)+4;       // 독의 x좌표 랜덤 지정
    }while(eat_poison() || (poison[0]==food[0] && poison[1]==food[1]));         //뱀이 독을 먹었을 경우 OR 먹이 위치에 독이 생성되었을 경우 while문 실행을 통해 무작위로 독 생성
    
    move(poison[0],poison[1]);                  //해당 위치로 커서 이동
    addstr("X");                                //해당 커서의 위치에 독 출력
    
    refresh();                                  //update the screen
}
//뱀이 독을 먹었는지 확인하는 함수
int eat_poison(){
    int touch=0;
    
    if(x[0]+MAP_X==poison[1] && y[0]+MAP_Y==poison[0]){         //뱀의 위치가 독의 위치와 같을때 (뱀이 독을 먹었을 경우를 의미)
        move(poison[0],poison[1]);                              //해당 독의 위치로 이동하여 "  "를 삽입해 독을 지워줌
        addstr(" ");
        touch=1;                                                //뱀이 독을 먹었을 경우 touch에 1을 넣어 return해줌
    }
    
    refresh();                                  //update the screen

    return touch;
}
