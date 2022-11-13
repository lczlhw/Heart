#define _USE_MATH_DEFINES


#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define CANVAS_WIDTH 640
#define CANVAS_HEIGHT 480
#define CANVAS_CENTER_X (CANVAS_WIDTH / 2)
#define CANVAS_CENTER_Y (CANVAS_HEIGHT / 2)
#define IMAGE_ENLARGE 8
#define FRAME 20
#define SPEED 50
#define ID_TIMER 1

struct Point{
	int x;
	int y;
	int size;
};
void copyPoint(struct Point *p1, struct Point *p2);

struct Set{
	struct Element{
		struct Point p;
		struct Element *next;
	};
	struct Element *header;
	struct Element *iter;
};
void initSet(struct Set *set);
void initIter(struct Set *set);
bool pushSet(struct Set *set, struct Point *p);
bool findSet(struct Set *set, int i, struct Point *p);
bool iterSet(struct Set *set, struct Point *p);
int lengthSet(struct Set *set);
void deleteSet(struct Set *set);

struct Heart{
	struct Set setPoints, setExPoints, setInPoints, setOutPoints;
	struct Set setAllPoints[FRAME];
	COLORREF color;
};
void initHeart(struct Heart *h);
void buildHeart(struct Heart *h, int angle);
void calc(struct Heart *h, int frame);
void clearHeart(struct Heart *h);
void deleteHeart(struct Heart *h);
void render(struct Heart *h, HDC hdc, int frame);

float random();
int randint(int a, int b);
float uniform(int a, int b);
void choice(struct Set *set, struct Point *p);

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void heart_func(float t, struct Point *p);
float curve(float angle);
void scater_inside(struct Point *p, float beta);

// func
// class Point//start
inline void copyPoint(struct Point *p1, struct Point *p2){
	p1->x = p2->x;
	p1->y = p2->y;
	p1->size = p2->size;
}

void calc_position(struct Point *p, float ratio){
	float force = 1.0f / pow(pow(p->x - CANVAS_CENTER_X, 2) + pow(p->y - CANVAS_CENTER_Y, 2), 0.5);
	float dx = ratio * force * (p->x - CANVAS_CENTER_X) + randint(-1, 1);
	float dy = ratio * force * (p->y - CANVAS_CENTER_Y) + randint(-1, 1);
	p->x = (int)(p->x - dx);
	p->y = (int)(p->y - dy);
}
// class Point//end

// class Set//start
void initSet(struct Set *set){
	struct Element *head = (struct Element*)malloc(sizeof(struct Element));
	head->p.size = 0;
	head->next = NULL;
	set->header = head;
	set->iter = set->header;
}
void initIter(struct Set *set){
	set->iter = set->header;
}

bool pushSet(struct Set *set, struct Point *p){
	struct Element *next = set->header;
	struct Element *more = (struct Element*)malloc(sizeof(struct Element));

	if (more == NULL){
		return false;
	}
	copyPoint(&(more->p), p);
	more->next = next->next;
	next->next = more;

	set->header->p.size++;
	return true;
}

bool findSet(struct Set *set, int i, struct Point *p){
	int size = set->header->p.size;
	struct Element *next = set->header->next;

	if (i >= size){
		return false;
	}

	for (; i > 0; i--){
		next = next->next;
	}

	copyPoint(p, &(next->p));
	return true;
}

bool iterSet(struct Set *set, struct Point *p){
	set->iter = set->iter->next;

	if (set->iter != NULL){
		copyPoint(p, &(set->iter->p));
		return true;
	}

	return false;
}

inline int lengthSet(struct Set *set){
	return set->header->p.size;
}

void deleteSet(struct Set *set){
	struct Element *next = set->header;
	struct Element *temp;

	while (next != NULL){
		temp = next;
		next = next->next;
		free(temp);
	}
	set->header = NULL;
}
// class Set//end

// class Heart//start
void initHeart(struct Heart *h){
	int i;
	initSet(&(h->setPoints));
	initSet(&(h->setExPoints));
	initSet(&(h->setInPoints));
	initSet(&(h->setOutPoints));
	h->color = RGB(255, 33, 33);
	buildHeart(h, 4000);

	clearHeart(h);
}

void buildHeart(struct Heart *h, int angle){
	int i;
	struct Point p;
    for (i = 0; i < angle; i++){
        //heart_func(2 * M_PI * i / angle, &p);
    	heart_func(uniform(0, 2 * M_PI), &p);
        pushSet(&(h->setPoints), &p);
    }

    while (iterSet(&(h->setPoints), &p)){
        for (i = 0; i < 3; i++){
            scater_inside(&p, 0.05f);
            pushSet(&(h->setExPoints), &p);
        }
    }

    for (i = 0; i < angle; i++){
        choice(&(h->setPoints), &p);
        scater_inside(&p, 0.15f);
        pushSet(&(h->setInPoints), &p);
    }

	for (i = 0; i < angle; i++){
    	heart_func(uniform(0, 2 * M_PI), &p);
        pushSet(&(h->setOutPoints), &p);
    }


    for (i = 0; i < FRAME; i++){
		calc(h, i);
	}
}

void calc(struct Heart *h, int frame){
	float ratio = curve(frame / 20.0);
	float in_ratio = 10 * ratio;
	struct Set all_point;
	struct Point p;

	initSet(&all_point);

	initIter(&(h->setPoints));
	while (iterSet(&(h->setPoints), &p)){
		calc_position(&p, in_ratio);
		p.size = randint(1, 3);
		pushSet(&all_point, &p);
	}

	initIter(&(h->setExPoints));
	while (iterSet(&(h->setExPoints), &p)){
		calc_position(&p, in_ratio);
		p.size = randint(1, 2);
		pushSet(&all_point, &p);
	}

	initIter(&(h->setInPoints));
	while (iterSet(&(h->setInPoints), &p)){
		calc_position(&p, in_ratio);
		p.size = randint(1, 2);
		pushSet(&all_point, &p);
	}

	float out_ratio = -(4 + 6 * (1 + ratio));
	int out_number = (int)(1000 + 4000 * pow(ratio, 2));
	int i;

	for (i = 0; i < out_number; i++){
		choice(&(h->setOutPoints), &p);
		calc_position(&p, out_ratio);
		p.x += randint(-14, 14);
		p.y += randint(-14, 14);
		p.size = randint(0, 2)? randint(1, 3): 2;
		pushSet(&all_point, &p);
	}
	h->setAllPoints[frame].header = all_point.header;
}

void clearHeart(struct Heart *h){
	deleteSet(&(h->setPoints));
	deleteSet(&(h->setExPoints));
	deleteSet(&(h->setInPoints));
	deleteSet(&(h->setOutPoints));
}

void deleteHeart(struct Heart *h){
	int i;
	for (i = 0; i < FRAME; i++){
		deleteSet(&(h->setAllPoints[i]));
	}
}

void render(struct Heart *h, HDC hdc, int frame){
	struct Point p;
	int i, j;
	frame %= FRAME;
	initIter(&(h->setAllPoints[frame]));
	while (iterSet(&(h->setAllPoints[frame]), &p)){
    	for (i = p.x; i < p.x+p.size; i++){
    		for (j = p.y; j < p.y+p.size; j++){
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
				SetPixelV(hdc, i, j, h->color);
#else
				SetPixel(hdc, i, j, h->color);
#endif
			}
		}
    }
}
// class Heart//end

// rand//start
inline float random(){
	float a = (float)rand();
	float b = (float)rand();
	return (float)(a < b? a / b: b / a);
}

inline int randint(int a, int b){
	return b > a? (int)(rand()%(b-a)+a): 0;
}

inline float uniform(int a, int b){
	return (float)(randint(a, b) + random());
}

inline void choice(struct Set *set, struct Point *p){
	findSet(set, randint(0, lengthSet(set)), p);
}
// rand//end

// create heart
void heart_func(float t, struct Point *p){
    float x;
    float y;
	x = 16 * pow(sin(t), 3);
	y = -(13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t));

    x *= IMAGE_ENLARGE;
    y *= IMAGE_ENLARGE;

    x += CANVAS_CENTER_X;
    y += CANVAS_CENTER_Y;

    p->x = x;
    p->y = y;
}

// move speed
inline float curve(float angle){
    // return (float)(sin(angle * 2 * pi));
    return (float)(2 * (2 * sin(angle * 2 * M_PI)) / (2 * M_PI));
}

// move point
void scater_inside(struct Point *p, float beta){
    float ratiox = -beta * log(random());
    float ratioy = -beta * log(random());
    int dx = (int)(ratiox * (p->x - CANVAS_CENTER_X));
    int dy = (int)(ratioy * (p->y - CANVAS_CENTER_Y));
    p->x -= dx;
    p->y -= dy;
}

struct Heart h;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("HeartWindow");
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, "Window Registration Failed!", szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(
			szAppName, TEXT("Heart"),
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, CANVAS_WIDTH, CANVAS_HEIGHT,
            NULL,
            NULL,
            hInstance,
            NULL);

    if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", szAppName, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    static HDC hdcBuffer[FRAME];
    static HBITMAP memBM[FRAME];
    PAINTSTRUCT ps;
    static RECT rect;
    static int frame;
    int i;

    switch (message)
    {
    	case WM_CREATE:
		    srand((unsigned int)time(NULL));

			initHeart(&h);
			frame = 0;

			hdc = GetDC(hwnd);
			GetClientRect(hwnd, &rect);
			for (i = 0; i < FRAME; i++){
				hdcBuffer[i] = CreateCompatibleDC(hdc);
				memBM[i] = CreateCompatibleBitmap(hdc, rect.right-rect.left, rect.bottom-rect.top);
				SelectObject(hdcBuffer[i], memBM[i]);
				
				FillRect(hdcBuffer[i], &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
				render(&h, hdcBuffer[i], i);
			}
			DeleteDC(hdc);
			
			SetTimer(hwnd, ID_TIMER, SPEED, NULL);
			break;

    	case WM_TIMER:
    		frame++;
    		if (frame >= FRAME){
				frame = 0;
			}
    		InvalidateRect(hwnd, NULL, TRUE);
    		break;

		case WM_ERASEBKGND:
			break;
			
    	case WM_PAINT:
    		hdc = BeginPaint(hwnd, &ps);

			BitBlt(hdc, 0, 0, rect.right-rect.left, rect.bottom-rect.top, hdcBuffer[frame], 0, 0, SRCCOPY);

    		EndPaint(hwnd, &ps);
            return 0;

        case WM_DESTROY:
        	for (i = 0; i < FRAME; i++){
				DeleteDC(hdcBuffer[i]);
				DeleteObject(memBM[i]);
			}
        	KillTimer(hwnd, ID_TIMER);
        	deleteHeart(&h);
            PostQuitMessage(0);
            return 0;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
    }
}


