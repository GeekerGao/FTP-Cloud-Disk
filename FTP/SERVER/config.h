// 定义常量
#define LS   0   // 常量 LS，表示列出目录内容
#define GET  1   // 常量 GET，表示从服务端获取文件到客户端
#define PWD  2   // 常量 PWD，表示显示当前工作目录
 
#define IFGO 3   // 常量 IFGO，用于判断命令是否需要进一步处理
 
#define LCD  4   // 常量 LCD，表示切换本地工作目录
#define LLS  5   // 常量 LLS，表示在本地执行 ls 命令
#define CD   6   // 常量 CD，表示切换服务器工作目录
#define PUT  7   // 常量 PUT，表示从客户端上传文件到服务端
 
#define QUIT   8   // 常量 QUIT，表示退出客户端或服务器
#define DOFILE 9   // 常量 DOFILE，表示传输文件内容
 
// 定义消息结构体 Msg
struct Msg
{
    int type;                 // 消息类型，表示不同的命令或操作
    char data[1024];          // 存储消息数据，如命令、消息内容等
    char secondBuf[128];      // 存储消息中的第二部分数据，如文件内容等
};