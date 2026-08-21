/*
 * Exact freestanding Win64 source used to build dist/ThanLongHiddenAutoChat_TEST.exe.
 * Purpose: standalone background chat compatibility test before semantic Lua/Network integration.
 *
 * This source deliberately avoids CRT dependencies and declares only the WinAPI surface it needs.
 * It scans visible processes for GameAssembly.dll, lets the user select one window, enter text,
 * interval and repeat count, then sends VK_RETURN / WM_CHAR / VK_RETURN directly to that HWND.
 * No SetForegroundWindow, SendInput, mouse movement/click, or clipboard is used.
 */

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD32;
typedef unsigned long long ULONG_PTR;
typedef long long LONG_PTR;
typedef unsigned long long WPARAM;
typedef long long LPARAM;
typedef long long LRESULT;
typedef int BOOL;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef void* HMENU;
typedef unsigned short WCHAR;
typedef const WCHAR* LPCWSTR;
typedef WCHAR* LPWSTR;
typedef unsigned long long UINT_PTR;
#define WINAPI __stdcall
#define CALLBACK __stdcall
#define TRUE 1
#define FALSE 0
#define NULL ((void*)0)
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define MAX_PATH 260
#define TH32CS_SNAPMODULE 0x00000008
#define TH32CS_SNAPMODULE32 0x00000010
#define WS_OVERLAPPED 0x00000000L
#define WS_CAPTION 0x00C00000L
#define WS_SYSMENU 0x00080000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_CHILD 0x40000000L
#define WS_VISIBLE 0x10000000L
#define WS_VSCROLL 0x00200000L
#define WS_EX_CLIENTEDGE 0x00000200L
#define CBS_DROPDOWNLIST 0x0003L
#define ES_MULTILINE 0x0004L
#define ES_AUTOVSCROLL 0x0040L
#define ES_NUMBER 0x2000L
#define BS_PUSHBUTTON 0x00000000L
#define BS_DEFPUSHBUTTON 0x00000001L
#define CW_USEDEFAULT ((int)0x80000000)
#define SW_SHOW 5
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_COMMAND 0x0111
#define WM_TIMER 0x0113
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define CB_ADDSTRING 0x0143
#define CB_RESETCONTENT 0x014B
#define CB_GETCURSEL 0x0147
#define CB_SETCURSEL 0x014E
#define VK_RETURN 0x0D
#define MAPVK_VK_TO_VSC 0
#define MB_ICONWARNING 0x00000030L
#define IDC_ARROW ((LPCWSTR)32512)
#define IDI_APPLICATION ((LPCWSTR)32512)
#define COLOR_WINDOW 5
#define IDC_GAME_LIST 1001
#define IDC_SCAN 1002
#define IDC_MESSAGE 1003
#define IDC_INTERVAL 1004
#define IDC_REPEAT 1005
#define IDC_START 1006
#define IDC_STOP 1007
#define IDC_STATUS 1008
#define TIMER_SEND 1
#define LOWORD(l) ((WORD)((ULONG_PTR)(l) & 0xffff))

#pragma pack(push,8)
typedef struct { DWORD32 dwSize; DWORD32 th32ModuleID; DWORD32 th32ProcessID; DWORD32 GlblcntUsage; DWORD32 ProccntUsage; BYTE* modBaseAddr; DWORD32 modBaseSize; void* hModule; WCHAR szModule[256]; WCHAR szExePath[MAX_PATH]; } MODULEENTRY32W;
typedef struct { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD32 time; long pt_x; long pt_y; DWORD32 lPrivate; } MSG;
typedef LRESULT (CALLBACK *WNDPROC)(HWND,UINT,WPARAM,LPARAM);
typedef struct { UINT cbSize; UINT style; WNDPROC lpfnWndProc; int cbClsExtra; int cbWndExtra; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCWSTR lpszMenuName; LPCWSTR lpszClassName; HICON hIconSm; } WNDCLASSEXW;
#pragma pack(pop)
typedef BOOL (CALLBACK *WNDENUMPROC)(HWND, LPARAM);

__declspec(dllimport) HANDLE WINAPI CreateToolhelp32Snapshot(DWORD32,DWORD32);
__declspec(dllimport) BOOL WINAPI Module32FirstW(HANDLE,MODULEENTRY32W*);
__declspec(dllimport) BOOL WINAPI Module32NextW(HANDLE,MODULEENTRY32W*);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) void WINAPI ExitProcess(UINT);
__declspec(dllimport) HINSTANCE WINAPI GetModuleHandleW(LPCWSTR);
__declspec(dllimport) BOOL WINAPI EnumWindows(WNDENUMPROC,LPARAM);
__declspec(dllimport) BOOL WINAPI IsWindowVisible(HWND);
__declspec(dllimport) int WINAPI GetWindowTextLengthW(HWND);
__declspec(dllimport) DWORD32 WINAPI GetWindowThreadProcessId(HWND,DWORD32*);
__declspec(dllimport) int WINAPI GetWindowTextW(HWND,LPWSTR,int);
__declspec(dllimport) HWND WINAPI CreateWindowExW(DWORD32,LPCWSTR,LPCWSTR,DWORD32,int,int,int,int,HWND,HMENU,HINSTANCE,void*);
__declspec(dllimport) LRESULT WINAPI DefWindowProcW(HWND,UINT,WPARAM,LPARAM);
__declspec(dllimport) unsigned short WINAPI RegisterClassExW(const WNDCLASSEXW*);
__declspec(dllimport) BOOL WINAPI ShowWindow(HWND,int);
__declspec(dllimport) BOOL WINAPI UpdateWindow(HWND);
__declspec(dllimport) BOOL WINAPI GetMessageW(MSG*,HWND,UINT,UINT);
__declspec(dllimport) BOOL WINAPI TranslateMessage(const MSG*);
__declspec(dllimport) LRESULT WINAPI DispatchMessageW(const MSG*);
__declspec(dllimport) void WINAPI PostQuitMessage(int);
__declspec(dllimport) BOOL WINAPI PostMessageW(HWND,UINT,WPARAM,LPARAM);
__declspec(dllimport) LRESULT WINAPI SendMessageW(HWND,UINT,WPARAM,LPARAM);
__declspec(dllimport) BOOL WINAPI SetWindowTextW(HWND,LPCWSTR);
__declspec(dllimport) int WINAPI MessageBoxW(HWND,LPCWSTR,LPCWSTR,UINT);
__declspec(dllimport) UINT_PTR WINAPI SetTimer(HWND,UINT_PTR,UINT,void*);
__declspec(dllimport) BOOL WINAPI KillTimer(HWND,UINT_PTR);
__declspec(dllimport) BOOL WINAPI EnableWindow(HWND,BOOL);
__declspec(dllimport) BOOL WINAPI IsWindow(HWND);
__declspec(dllimport) UINT WINAPI MapVirtualKeyW(UINT,UINT);
__declspec(dllimport) HCURSOR WINAPI LoadCursorW(HINSTANCE,LPCWSTR);
__declspec(dllimport) HICON WINAPI LoadIconW(HINSTANCE,LPCWSTR);

static const WCHAR S_TITLE[] = u"Thần Long Hidden Auto Chat - TEST";
static const WCHAR S_GAMEASSEMBLY[] = u"GameAssembly.dll";
static const WCHAR S_CLASS[] = u"ThanLongHiddenAutoChatTest";
static const WCHAR S_STATIC[] = u"STATIC";
static const WCHAR S_BUTTON[] = u"BUTTON";
static const WCHAR S_EDIT[] = u"EDIT";
static const WCHAR S_COMBO[] = u"COMBOBOX";
static const WCHAR S_SCAN[] = u"Quét lại";
static const WCHAR S_START[] = u"Bắt đầu";
static const WCHAR S_STOP[] = u"Dừng";
static const WCHAR S_GAMEWIN[] = u"Cửa sổ game";
static const WCHAR S_CHAT[] = u"Nội dung chat";
static const WCHAR S_INTERVAL[] = u"Khoảng lặp (giây)";
static const WCHAR S_REPEAT[] = u"Số lần";
static const WCHAR S_READY[] = u"Sẵn sàng.";
static const WCHAR S_NONE[] = u"Không tìm thấy cửa sổ có GameAssembly.dll.";
static const WCHAR S_SELECT[] = u"Hãy quét và chọn một cửa sổ game.";
static const WCHAR S_EMPTY[] = u"Nội dung chat đang trống.";
static const WCHAR S_DONE[] = u"Hoàn tất số lần lặp.";
static const WCHAR S_STOPPED[] = u"Đã dừng thủ công.";
static const WCHAR S_CLOSED[] = u"Cửa sổ game đã đóng; đã dừng.";
static const WCHAR S_FAILED[] = u"Gửi chat thất bại; đã dừng.";
static const WCHAR S_DEFAULT_INT[] = u"10";
static const WCHAR S_DEFAULT_REPEAT[] = u"5";

typedef struct { DWORD32 pid; DWORD32 tid; HWND hwnd; WCHAR title[256]; } GAMEWIN;
static HINSTANCE g_inst; static HWND g_main,g_list,g_scan,g_message,g_interval,g_repeat,g_start,g_stop,g_status;
static GAMEWIN g_games[64]; static int g_game_count; static GAMEWIN g_target; static int g_remaining; static UINT g_interval_ms; static BOOL g_running;

static void zero_mem(void* p, unsigned long long n){ BYTE* b=(BYTE*)p; while(n--) *b++=0; }
void* memcpy(void* d,const void* s,unsigned long long n){ BYTE* dd=(BYTE*)d; const BYTE* ss=(const BYTE*)s; while(n--) *dd++=*ss++; return d; }
static int wlen(const WCHAR* s){ int n=0; if(!s) return 0; while(s[n]) n++; return n; }
static int weq_i(const WCHAR* a,const WCHAR* b){ int i=0; WCHAR x,y; for(;;i++){ x=a[i];y=b[i]; if(x>=u'A'&&x<=u'Z')x+=32; if(y>=u'A'&&y<=u'Z')y+=32; if(x!=y)return 0; if(!x)return 1; } }
static void wappend(WCHAR* d,int cap,const WCHAR* s){ int n=wlen(d),i=0; while(n+1<cap&&s&&s[i]) d[n++]=s[i++]; d[n]=0; }
static void append_u32(WCHAR* d,int cap,DWORD32 v){ WCHAR t[16];int n=0,i;if(v==0){wappend(d,cap,u"0");return;}while(v&&n<15){t[n++]=(WCHAR)(u'0'+(v%10));v/=10;}for(i=n-1;i>=0;i--){WCHAR q[2]={t[i],0};wappend(d,cap,q);} }
static int parse_pos(HWND h,int fallback,int minv,int maxv){ WCHAR b[32];int i=0,v=0; zero_mem(b,sizeof(b)); GetWindowTextW(h,b,31); if(!b[0])return fallback; while(b[i]){ if(b[i]<u'0'||b[i]>u'9')return fallback; if(v>100000000)return fallback; v=v*10+(b[i]-u'0');i++; } if(v<minv||v>maxv)return fallback; return v; }
static void set_status(const WCHAR* s){ SetWindowTextW(g_status,s); }
static void get_text(HWND h,WCHAR* out,int cap){ zero_mem(out,cap*2); GetWindowTextW(h,out,cap); }
static BOOL has_module(DWORD32 pid){ HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid); MODULEENTRY32W me; if(s==INVALID_HANDLE_VALUE)return FALSE; zero_mem(&me,sizeof(me)); me.dwSize=sizeof(me); if(Module32FirstW(s,&me)){ do{ if(weq_i(me.szModule,S_GAMEASSEMBLY)){CloseHandle(s);return TRUE;} }while(Module32NextW(s,&me)); } CloseHandle(s); return FALSE; }
static BOOL CALLBACK enum_proc(HWND h,LPARAM lp){ DWORD32 pid=0,tid;int i,len; (void)lp; if(!IsWindowVisible(h))return TRUE; len=GetWindowTextLengthW(h); if(len<=0)return TRUE; tid=GetWindowThreadProcessId(h,&pid); if(!pid||!tid||!has_module(pid))return TRUE; for(i=0;i<g_game_count;i++)if(g_games[i].pid==pid)return TRUE; if(g_game_count>=64)return TRUE; g_games[g_game_count].pid=pid;g_games[g_game_count].tid=tid;g_games[g_game_count].hwnd=h;zero_mem(g_games[g_game_count].title,sizeof(g_games[g_game_count].title));GetWindowTextW(h,g_games[g_game_count].title,255);g_game_count++;return TRUE; }
static void scan_games(void){ int i;WCHAR line[512],status[128]; g_game_count=0;zero_mem(g_games,sizeof(g_games));SendMessageW(g_list,CB_RESETCONTENT,0,0);EnumWindows(enum_proc,0); for(i=0;i<g_game_count;i++){zero_mem(line,sizeof(line));wappend(line,512,u"PID ");append_u32(line,512,g_games[i].pid);wappend(line,512,u" | ");wappend(line,512,g_games[i].title);SendMessageW(g_list,CB_ADDSTRING,0,(LPARAM)line);} if(g_game_count){SendMessageW(g_list,CB_SETCURSEL,0,0);zero_mem(status,sizeof(status));wappend(status,128,u"Đã quét thấy ");append_u32(status,128,(DWORD32)g_game_count);wappend(status,128,u" cửa sổ game.");set_status(status);}else set_status(S_NONE); }
static BOOL post_key(HWND h,UINT vk){ UINT sc;LPARAM down,up;if(!IsWindow(h))return FALSE;sc=MapVirtualKeyW(vk,MAPVK_VK_TO_VSC);down=(LPARAM)(1ULL|((unsigned long long)sc<<16));up=down|((LPARAM)1<<30)|((LPARAM)1<<31);return PostMessageW(h,WM_KEYDOWN,vk,down)&&PostMessageW(h,WM_KEYUP,vk,up); }
static BOOL post_text(HWND h,const WCHAR* s){ int i=0;if(!IsWindow(h))return FALSE;while(s[i]){WCHAR c=s[i++];if(c==u'\r'||c==u'\n')continue;if(!PostMessageW(h,WM_CHAR,(WPARAM)c,1))return FALSE;}return TRUE; }
static BOOL send_once(const GAMEWIN* g,const WCHAR* m){ if(!g||!IsWindow(g->hwnd)||!m||!m[0])return FALSE; if(!post_key(g->hwnd,VK_RETURN))return FALSE; if(!post_text(g->hwnd,m))return FALSE; if(!post_key(g->hwnd,VK_RETURN))return FALSE; return TRUE; }
static void stop_run(const WCHAR* reason){KillTimer(g_main,TIMER_SEND);g_running=FALSE;EnableWindow(g_start,TRUE);EnableWindow(g_scan,TRUE);EnableWindow(g_list,TRUE);EnableWindow(g_stop,FALSE);if(reason)set_status(reason);}
static void start_run(void){ int sel,interval;WCHAR msg[512]; if(g_running)return;sel=(int)SendMessageW(g_list,CB_GETCURSEL,0,0);if(sel<0||sel>=g_game_count){MessageBoxW(g_main,S_SELECT,S_TITLE,MB_ICONWARNING);return;}get_text(g_message,msg,512);if(!msg[0]){MessageBoxW(g_main,S_EMPTY,S_TITLE,MB_ICONWARNING);return;}g_target=g_games[sel];interval=parse_pos(g_interval,10,1,86400);g_interval_ms=(UINT)(interval*1000);g_remaining=parse_pos(g_repeat,1,1,1000000);g_running=TRUE;EnableWindow(g_start,FALSE);EnableWindow(g_scan,FALSE);EnableWindow(g_list,FALSE);EnableWindow(g_stop,TRUE);if(!send_once(&g_target,msg)){stop_run(S_FAILED);return;}g_remaining--;if(g_remaining<=0){stop_run(S_DONE);return;}SetTimer(g_main,TIMER_SEND,g_interval_ms,NULL);set_status(u"Đã gửi lần đầu; đang chờ lần kế tiếp.");}
static void on_timer(void){ WCHAR msg[512]; if(!g_running)return;if(!IsWindow(g_target.hwnd)){stop_run(S_CLOSED);return;}get_text(g_message,msg,512);if(!msg[0]){stop_run(S_EMPTY);return;}if(!send_once(&g_target,msg)){stop_run(S_FAILED);return;}g_remaining--;if(g_remaining<=0)stop_run(S_DONE);else set_status(u"Đã gửi; đang chờ lần kế tiếp.");}
static LRESULT CALLBACK wndproc(HWND h,UINT m,WPARAM w,LPARAM l){(void)l;switch(m){case WM_CREATE:g_list=CreateWindowExW(0,S_COMBO,u"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,14,30,470,260,h,(HMENU)(ULONG_PTR)IDC_GAME_LIST,g_inst,NULL);g_scan=CreateWindowExW(0,S_BUTTON,S_SCAN,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,500,30,90,28,h,(HMENU)(ULONG_PTR)IDC_SCAN,g_inst,NULL);CreateWindowExW(0,S_STATIC,S_GAMEWIN,WS_CHILD|WS_VISIBLE,14,10,120,18,h,NULL,g_inst,NULL);CreateWindowExW(0,S_STATIC,S_CHAT,WS_CHILD|WS_VISIBLE,14,68,120,18,h,NULL,g_inst,NULL);g_message=CreateWindowExW(WS_EX_CLIENTEDGE,S_EDIT,u"",WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,14,88,576,90,h,(HMENU)(ULONG_PTR)IDC_MESSAGE,g_inst,NULL);CreateWindowExW(0,S_STATIC,S_INTERVAL,WS_CHILD|WS_VISIBLE,14,190,130,18,h,NULL,g_inst,NULL);CreateWindowExW(0,S_STATIC,S_REPEAT,WS_CHILD|WS_VISIBLE,154,190,90,18,h,NULL,g_inst,NULL);g_interval=CreateWindowExW(WS_EX_CLIENTEDGE,S_EDIT,S_DEFAULT_INT,WS_CHILD|WS_VISIBLE|ES_NUMBER,14,210,110,26,h,(HMENU)(ULONG_PTR)IDC_INTERVAL,g_inst,NULL);g_repeat=CreateWindowExW(WS_EX_CLIENTEDGE,S_EDIT,S_DEFAULT_REPEAT,WS_CHILD|WS_VISIBLE|ES_NUMBER,154,210,110,26,h,(HMENU)(ULONG_PTR)IDC_REPEAT,g_inst,NULL);g_start=CreateWindowExW(0,S_BUTTON,S_START,WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,294,208,90,30,h,(HMENU)(ULONG_PTR)IDC_START,g_inst,NULL);g_stop=CreateWindowExW(0,S_BUTTON,S_STOP,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,394,208,90,30,h,(HMENU)(ULONG_PTR)IDC_STOP,g_inst,NULL);EnableWindow(g_stop,FALSE);g_status=CreateWindowExW(0,S_STATIC,S_READY,WS_CHILD|WS_VISIBLE,14,252,576,46,h,(HMENU)(ULONG_PTR)IDC_STATUS,g_inst,NULL);scan_games();return 0;case WM_COMMAND:switch(LOWORD(w)){case IDC_SCAN:scan_games();return 0;case IDC_START:start_run();return 0;case IDC_STOP:stop_run(S_STOPPED);return 0;}break;case WM_TIMER:if(w==TIMER_SEND){on_timer();return 0;}break;case WM_DESTROY:KillTimer(h,TIMER_SEND);PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
void wWinMainCRTStartup(void){WNDCLASSEXW wc;MSG msg;HWND h;zero_mem(&wc,sizeof(wc));zero_mem(&msg,sizeof(msg));g_inst=GetModuleHandleW(NULL);wc.cbSize=sizeof(wc);wc.lpfnWndProc=wndproc;wc.hInstance=g_inst;wc.hCursor=LoadCursorW(NULL,IDC_ARROW);wc.hIcon=LoadIconW(NULL,IDI_APPLICATION);wc.hbrBackground=(HBRUSH)(ULONG_PTR)(COLOR_WINDOW+1);wc.lpszClassName=S_CLASS;if(!RegisterClassExW(&wc))ExitProcess(1);h=CreateWindowExW(0,S_CLASS,S_TITLE,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,620,350,NULL,NULL,g_inst,NULL);if(!h)ExitProcess(2);g_main=h;ShowWindow(h,SW_SHOW);UpdateWindow(h);while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}ExitProcess((UINT)msg.wParam);}
