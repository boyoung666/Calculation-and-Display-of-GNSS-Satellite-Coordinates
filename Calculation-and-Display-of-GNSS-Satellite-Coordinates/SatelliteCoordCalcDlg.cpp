#include "pch.h"
#include "framework.h"
#include "SatelliteCoordCalc.h"
#include "SatelliteCoordCalcDlg.h"
#include "EnlargedViewDlg.h"
#include "afxdialogex.h"
#include "resource.h"  
#include "afxwin.h"
#include <ctime>
#include <sstream>
#include <map>
#include <Gdiplus.h>

using namespace Gdiplus;

// 调试模式宏定义，启用内存泄漏检测和调试信息
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// GDI+初始化配置
#include <gdiplus.h>
using namespace Gdiplus;
// 链接GDI+库，确保绘图功能正常编译
#pragma comment(lib, "gdiplus.lib")

// 关于对话框类定义（嵌套类，仅用于显示软件信息）
class CAboutDlg : public CDialogEx
{
public:
    // 构造函数：初始化关于对话框，指定对话框资源ID
    CAboutDlg();

    // 设计时资源ID声明（仅在设计模式生效）
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    // 数据交换函数：实现对话框与变量的绑定（此处无额外变量，仅调用父类）
    virtual void DoDataExchange(CDataExchange* pDX);
    // 消息映射声明：关联控件事件与处理函数
    DECLARE_MESSAGE_MAP()
public:
    // 时间编辑框内容变化事件（空实现，因关于对话框无需时间功能）
    afx_msg void OnEnChangeEditTime();
};

// 关于对话框构造函数实现
// 功能：初始化对话框，指定父类构造函数的资源ID
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

// 关于对话框数据交换函数实现
// 功能：调用父类数据交换逻辑，确保对话框基础功能正常
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

// 关于对话框消息映射实现
// 功能：将时间编辑框的内容变化事件绑定到处理函数
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_EN_CHANGE(IDC_EDIT_TIME, &CAboutDlg::OnEnChangeEditTime)
END_MESSAGE_MAP()

// 关于对话框时间编辑框事件处理函数
// 功能：空实现，因关于对话框的时间编辑框仅为占位，无需实际功能
void CAboutDlg::OnEnChangeEditTime()
{
    // 无需实现
}


//  CSatelliteCoordCalcDlg 类实现 
// 主对话框构造函数
// 功能：初始化对话框成员变量，设置默认值和初始状态
CSatelliteCoordCalcDlg::CSatelliteCoordCalcDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_SATELLITECOORDCALC_DIALOG, pParent)  // 调用父类构造函数，指定主对话框资源ID
    , m_bEphemerisLoaded(FALSE)                     // 初始化星历加载状态：未加载
    , m_pCoordsysImage(nullptr)                     // 初始化坐标系图片指针：空（未加载）
    , m_gdiplusToken(0)                             // 初始化GDI+令牌：0（未初始化）
    , m_nYear(2023)                                 // 初始化年份：2023
    , m_nMonth(9)                                   // 初始化月份：9（九月）
    , m_nDay(9)                                     // 初始化日期：9
    , m_nHour(0)                                    // 初始化小时：0
    , m_nMinute(0)                                   // 初始化分钟：0
    , m_dSecond(0.0)                                 // 初始化秒数：0.0
    , m_bAnyEphemerisLoaded(FALSE)                   // 初始化有效星历标记：无有效星历
    , m_dStationX(0.0)                                // 初始化测站X坐标：0.0m
    , m_dStationY(0.0)                                // 初始化测站Y坐标：0.0m
    , m_dStationZ(0.0)                                // 初始化测站Z坐标：0.0m
{
    // 加载主窗口图标（从项目资源中获取）
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// 数据交换函数
// 功能：实现对话框控件与成员变量的双向绑定，包含数据验证
// 参数：pDX - 数据交换对象，用于传递绑定信息
void CSatelliteCoordCalcDlg::DoDataExchange(CDataExchange* pDX)
{
    // 调用父类数据交换逻辑，确保基础功能正常
    CDialogEx::DoDataExchange(pDX);

    //  时间字段绑定与验证 
    // 年份编辑框（IDC_EDIT_YEAR）与m_nYear绑定
    DDX_Text(pDX, IDC_EDIT_YEAR, m_nYear);
    // 月份编辑框（IDC_EDIT_MONTH）与m_nMonth绑定
    DDX_Text(pDX, IDC_EDIT_MONTH, m_nMonth);
    // 日期编辑框（IDC_EDIT_DAY）与m_nDay绑定
    DDX_Text(pDX, IDC_EDIT_DAY, m_nDay);
    // 小时编辑框（IDC_EDIT_HOUR）与m_nHour绑定
    DDX_Text(pDX, IDC_EDIT_HOUR, m_nHour);
    // 分钟编辑框（IDC_EDIT_MINUTE）与m_nMinute绑定
    DDX_Text(pDX, IDC_EDIT_MINUTE, m_nMinute);
    // 秒数编辑框（IDC_EDIT_SECOND）与m_dSecond绑定
    DDX_Text(pDX, IDC_EDIT_SECOND, m_dSecond);

    // 时间字段数据验证（确保输入在合法范围）
    DDV_MinMaxInt(pDX, m_nYear, 2000, 2100);       // 年份范围：2000-2100
    DDV_MinMaxInt(pDX, m_nMonth, 1, 12);           // 月份范围：1-12
    DDV_MinMaxInt(pDX, m_nDay, 1, 31);             // 日期范围：1-31（简化验证，未处理月份天数差异）
    DDV_MinMaxInt(pDX, m_nHour, 0, 23);            // 小时范围：0-23
    DDV_MinMaxInt(pDX, m_nMinute, 0, 59);          // 分钟范围：0-59
    DDV_MinMaxDouble(pDX, m_dSecond, 0.0, 59.999); // 秒数范围：0.0-59.999（支持毫秒级精度）

    // 控件绑定
    // 结果列表控件（IDC_LIST_RESULT）与m_listResult绑定（用于显示卫星坐标）
    DDX_Control(pDX, IDC_LIST_RESULT, m_listResult);
    // 坐标系图片控件（IDC_STATIC_COORDSYS）与m_staticCoordsys绑定（用于绘制卫星图）
    DDX_Control(pDX, IDC_STATIC_COORDSYS, m_staticCoordsys);
    // 单个卫星PRN编辑框（IDC_EDIT_SINGLE_PRN）与m_strSinglePRN绑定（用于输入单个卫星PRN）
    DDX_Text(pDX, IDC_EDIT_SINGLE_PRN, m_strSinglePRN);

    // 测站坐标绑定与验证
    // 测站X坐标编辑框（IDC_EDIT_STATION_X）与m_dStationX绑定
    DDX_Text(pDX, IDC_EDIT_STATION_X, m_dStationX);
    // 测站Y坐标编辑框（IDC_EDIT_STATION_Y）与m_dStationY绑定
    DDX_Text(pDX, IDC_EDIT_STATION_Y, m_dStationY);
    // 测站Z坐标编辑框（IDC_EDIT_STATION_Z）与m_dStationZ绑定
    DDX_Text(pDX, IDC_EDIT_STATION_Z, m_dStationZ);

    // 测站坐标数据验证（范围：±20000000m，覆盖地球表面所有可能坐标）
    DDV_MinMaxDouble(pDX, m_dStationX, -20000000.0, 20000000.0);
    DDV_MinMaxDouble(pDX, m_dStationY, -20000000.0, 20000000.0);
    DDV_MinMaxDouble(pDX, m_dStationZ, -20000000.0, 20000000.0);
}

// 消息映射实现
// 功能：将控件事件（如按钮点击、编辑框变化）与对应的处理函数绑定
BEGIN_MESSAGE_MAP(CSatelliteCoordCalcDlg, CDialogEx)
    ON_WM_SYSCOMMAND()                  // 系统命令（如窗口最大化、关于菜单）
    ON_WM_PAINT()                       // 窗口绘图事件（绘制图标、坐标系图片）
    ON_WM_QUERYDRAGICON()               // 窗口拖拽图标事件
    ON_BN_CLICKED(IDC_BTN_OPEN, &CSatelliteCoordCalcDlg::OnBnClickedBtnOpen)  // 打开星历文件按钮
    ON_BN_CLICKED(IDC_BTN_CALC, &CSatelliteCoordCalcDlg::OnBnClickedBtnCalc)  // 计算所有卫星坐标按钮
    ON_BN_CLICKED(IDC_BTN_SAVE, &CSatelliteCoordCalcDlg::OnBnClickedBtnSave)  // 保存计算结果按钮
    ON_BN_CLICKED(IDC_BTN_DRAW, &CSatelliteCoordCalcDlg::OnBnClickedBtnDraw)  // 绘制卫星轨道图按钮
    ON_WM_DESTROY()                     // 窗口销毁事件（释放资源）
    ON_EN_CHANGE(IDC_EDIT_YEAR, &CSatelliteCoordCalcDlg::OnEnChangeTimeFields)  // 年份编辑框变化
    ON_EN_CHANGE(IDC_EDIT_MONTH, &CSatelliteCoordCalcDlg::OnEnChangeTimeFields) // 月份编辑框变化
    ON_EN_CHANGE(IDC_EDIT_DAY, &CSatelliteCoordCalcDlg::OnEnChangeTimeFields)   // 日期编辑框变化
    ON_EN_CHANGE(IDC_EDIT_HOUR, &CSatelliteCoordCalcDlg::OnEnChangeTimeFields)  // 小时编辑框变化
    ON_EN_CHANGE(IDC_EDIT_MINUTE, &CSatelliteCoordCalcDlg::OnEnChangeTimeFields) // 分钟编辑框变化
    ON_EN_CHANGE(IDC_EDIT_SECOND, &CSatelliteCoordCalcDlg::OnEnChangeTimeFields) // 秒数编辑框变化
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RESULT, &CSatelliteCoordCalcDlg::OnLvnItemchangedListResult) // 列表项变化
    ON_EN_CHANGE(IDC_EDIT_STATION_X, &CSatelliteCoordCalcDlg::OnEnChangeStationCoords) // 测站X坐标变化
    ON_EN_CHANGE(IDC_EDIT_STATION_Y, &CSatelliteCoordCalcDlg::OnEnChangeStationCoords) // 测站Y坐标变化
    ON_EN_CHANGE(IDC_EDIT_STATION_Z, &CSatelliteCoordCalcDlg::OnEnChangeStationCoords) // 测站Z坐标变化
    ON_BN_CLICKED(IDC_BTN_CLEAR, &CSatelliteCoordCalcDlg::OnBnClickedBtnClear)  // 清除数据按钮
    ON_STN_CLICKED(IDC_STATIC_STATION_TITLE, &CSatelliteCoordCalcDlg::OnStnClickedStaticStationTitle) // 测站标题点击
    ON_STN_CLICKED(IDC_STATIC_COORDSYS, &CSatelliteCoordCalcDlg::OnStnClickedStaticCoordsys) // 坐标系控件点击
    ON_STN_CLICKED(IDC_STATIC_STATION_Z, &CSatelliteCoordCalcDlg::OnStnClickedStaticStationZ) // 测站Z坐标标题点击
    ON_BN_CLICKED(IDC_BTN_CALC_SINGLE, &CSatelliteCoordCalcDlg::OnBnClickedBtnCalcSingle)  // 计算单个卫星坐标按钮
    ON_WM_LBUTTONDBLCLK()  // 鼠标左键双击事件（用于放大卫星图）
END_MESSAGE_MAP()

// 测站坐标编辑框内容变化处理函数
// 功能：实时更新测站坐标成员变量，确保变量与编辑框内容同步
void CSatelliteCoordCalcDlg::OnEnChangeStationCoords()
{
    // UpdateData(TRUE)：将编辑框内容读取到成员变量（m_dStationX/m_dStationY/m_dStationZ）
    UpdateData(TRUE);
}

// 时间字段（年/月/日/时/分/秒）内容变化处理函数
// 功能：实时更新时间成员变量，确保变量与编辑框内容同步
void CSatelliteCoordCalcDlg::OnEnChangeTimeFields()
{
    // UpdateData(TRUE)：将编辑框内容读取到时间成员变量（m_nYear/m_nMonth等）
    UpdateData(TRUE);
}

// 对话框初始化函数
// 功能：初始化对话框控件、加载资源、设置初始状态，返回初始化是否成功
BOOL CSatelliteCoordCalcDlg::OnInitDialog()
{
    //  初始化时间字段默认值 
    m_nYear = 2023;    // 默认年份：2023
    m_nMonth = 9;      // 默认月份：9
    m_nDay = 9;        // 默认日期：9
    m_nHour = 0;       // 默认小时：0
    m_nMinute = 0;     // 默认分钟：0
    m_dSecond = 0.0;   // 默认秒数：0.0

    // 初始化GDI+ 
    GdiplusStartupInput gdiplusInput;  // GDI+初始化输入参数（默认配置）
    // 启动GDI+，获取令牌（用于后续关闭GDI+）
    GdiplusStartup(&m_gdiplusToken, &gdiplusInput, NULL);

    // 调用父类初始化函数，确保对话框基础功能（如标题、图标）正常
    CDialogEx::OnInitDialog();

    // 初始化测站坐标（示例值） 
    // 使用BJFS站（北京房山站）的地心地固坐标作为默认值，方便测试
    m_dStationX = 4043581.524;  // BJFS站X坐标（m）
    m_dStationY = 117185.923;   // BJFS站Y坐标（m）
    m_dStationZ = 4915549.371;  // BJFS站Z坐标（m）

    // 加载坐标系背景图片 
    // 坐标系图片路径（相对路径）
    CString strImagePath = _T("coordsys.png");
    // 从文件加载图片，用于后续绘制卫星图背景
    m_pCoordsysImage = Image::FromFile(strImagePath.AllocSysString());

    // 图片加载错误处理：提示用户检查路径
    if (m_pCoordsysImage == nullptr || m_pCoordsysImage->GetLastStatus() != Ok)
    {
        CString msg;
        msg.Format(_T("坐标系图片加载失败！请检查路径：\n%s"), strImagePath);
        AfxMessageBox(msg);  // 弹出错误提示框
    }


    // 初始化星历加载状态
    m_bEphemerisLoaded = FALSE;  // 初始状态：未加载星历

    // 系统菜单添加"关于"选项 
    CMenu* pSysMenu = GetSystemMenu(FALSE);  // 获取系统菜单（窗口右上角的"最大化/最小化"菜单）
    if (pSysMenu != nullptr)
    {
        CString strAboutMenu;
        // 从资源加载"关于"菜单文本（支持多语言）
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);          // 添加分隔线
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);  // 添加"关于"菜单项
        }
    }

    //  初始化卫星类型-颜色映射 
    InitSatelliteColors();  // 为不同系统卫星（GPS、北斗等）分配颜色

    // 设置窗口图标
    SetIcon(m_hIcon, TRUE);   // 设置大图标（任务栏、窗口标题栏）
    SetIcon(m_hIcon, FALSE);  // 设置小图标（窗口左上角）

    // 初始化结果列表控件
    // 设置列表扩展样式：整行选中、显示网格线
    m_listResult.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    // 添加列表列：卫星PRN（宽度80，居中对齐）
    m_listResult.InsertColumn(0, _T("卫星PRN"), LVCFMT_CENTER, 80);
    // 添加列表列：X坐标（宽度120，居中对齐）
    m_listResult.InsertColumn(1, _T("X坐标（m）"), LVCFMT_CENTER, 120);
    // 添加列表列：Y坐标（宽度120，居中对齐）
    m_listResult.InsertColumn(2, _T("Y坐标（m）"), LVCFMT_CENTER, 120);
    // 添加列表列：Z坐标（宽度120，居中对齐）
    m_listResult.InsertColumn(3, _T("Z坐标（m）"), LVCFMT_CENTER, 120);

    // 初始化编辑框显示 
    // UpdateData(FALSE)：将成员变量值更新到编辑框（显示默认时间和测站坐标）
    UpdateData(FALSE);

    //  初始化有效星历标记 
    m_bAnyEphemerisLoaded = FALSE;  // 初始状态：无有效星历

    // 返回TRUE表示初始化成功
    return TRUE;
}

// 系统命令处理函数
// 功能：处理系统菜单命令（如"关于"、"关闭"），其他命令交给父类处理
void CSatelliteCoordCalcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    // 判断是否点击"关于"菜单（IDM_ABOUTBOX）
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;          // 创建关于对话框对象
        dlgAbout.DoModal();          // 以模态方式显示关于对话框（阻塞当前窗口）
    }
    else
    {
        // 其他系统命令（如最大化、最小化、关闭）交给父类处理
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 窗口绘图处理函数
// 功能：绘制窗口图标（最小化时）和坐标系背景图片，确保界面显示正常
void CSatelliteCoordCalcDlg::OnPaint()
{
    // 情况1：窗口处于最小化状态（绘制图标）
    if (IsIconic())
    {
        CPaintDC dc(this);  // 获取绘图设备上下文（自动管理资源）
        // 发送消息擦除图标背景，避免残留
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 获取系统图标尺寸（标准大小）
        int cxIcon = GetSystemMetrics(SM_CXICON);  // 图标宽度
        int cyIcon = GetSystemMetrics(SM_CYICON);  // 图标高度

        CRect rect;
        GetClientRect(&rect);  // 获取窗口客户区大小

        // 计算图标绘制位置（居中显示）
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标到窗口客户区
        dc.DrawIcon(x, y, m_hIcon);
    }
    // 情况2：窗口正常显示（绘制坐标系图片）
    else
    {
        // 调用父类绘图函数，确保窗口基础元素（如标题栏、边框）正常显示
        CDialogEx::OnPaint();

        // 绘制坐标系图片到指定控件（IDC_STATIC_COORDSYS）
        if (m_pCoordsysImage != nullptr && m_pCoordsysImage->GetLastStatus() == Ok)
        {
            // 获取坐标系图片控件的窗口指针
            CWnd* pStaticWnd = GetDlgItem(IDC_STATIC_COORDSYS);
            if (pStaticWnd == nullptr) return;  // 控件不存在则返回

            // 获取控件的设备上下文（用于绘图）
            CDC* pStaticDC = pStaticWnd->GetDC();
            CRect rectStatic;
            pStaticWnd->GetClientRect(&rectStatic);  // 获取控件客户区大小

            // 创建GDI+绘图对象，绑定到控件的设备上下文
            Graphics graphics(pStaticDC->m_hDC);
            // 绘制图片：拉伸图片以填充整个控件
            graphics.DrawImage(m_pCoordsysImage, 0, 0, rectStatic.Width(), rectStatic.Height());

            // 释放设备上下文（避免资源泄漏）
            pStaticWnd->ReleaseDC(pStaticDC);
        }
    }
}

// 窗口拖拽图标处理函数
// 功能：返回窗口拖拽时显示的图标，确保拖拽体验正常
HCURSOR CSatelliteCoordCalcDlg::OnQueryDragIcon()
{
    // 返回主窗口图标作为拖拽图标
    return static_cast<HCURSOR>(m_hIcon);
}

// D格式科学计数法转换函数
// 功能：将RINEX文件中的"D格式"（如1.234D+05）转换为C++支持的"e格式"（1.234e+05）
// 参数：str - 待转换的字符串（D格式科学计数法）
// 返回：转换后的双精度数值
double CSatelliteCoordCalcDlg::StrToDoubleWithD(const CString& str)
{
    CString strTemp = str;  // 复制输入字符串（避免修改原字符串）
    // 将字符'D'替换为'e'，适配C++的科学计数法解析
    strTemp.Replace(_T('D'), _T('e'));
    // _tstof：将宽字符字符串转换为双精度数值（兼容Unicode和多字节编码）
    return _tstof(strTemp);
}

// 组合时间字符串函数
// 功能：将年/月/日/时/分/秒组合为标准UTC时间字符串（格式：YYYY-MM-DD HH:MM:SS.sss）
// 返回：组合后的UTC时间字符串
CString CSatelliteCoordCalcDlg::CombineTimeString()
{
    CString strTime;
    // 格式化字符串：
    // %04d - 年份（4位，不足补0）
    // %02d - 月/日/时/分（2位，不足补0）
    // %.3f - 秒数（保留3位小数，支持毫秒级精度）
    strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%.3f"),
        m_nYear, m_nMonth, m_nDay,
        m_nHour, m_nMinute, m_dSecond);
    return strTime;
}

// UTC字符串转时间戳函数
// 功能：将标准UTC时间字符串（YYYY-MM-DD HH:MM:SS.sss）转换为时间戳（自1970-01-01 00:00:00以来的秒数）
// 参数：strUTC - 待转换的UTC时间字符串
// 返回：成功返回时间戳，失败返回-1
time_t CSatelliteCoordCalcDlg::StringToTime(const CString& strUTC)
{
    struct tm tmTime = { 0 };  // 初始化时间结构体（存储年月日时分秒）
    int nYear, nMonth, nDay, nHour, nMinute;  // 解析后的时间字段
    double dSecond;                          // 解析后的秒数（支持小数）

    // 解析UTC字符串：按"YYYY-MM-DD HH:MM:SS.sss"格式提取字段
    if (_stscanf_s(strUTC, _T("%d-%d-%d %d:%d:%lf"),
        &nYear, &nMonth, &nDay, &nHour, &nMinute, &dSecond) != 6)
    {
        return -1;  // 解析失败（格式不匹配），返回-1
    }

    // 填充时间结构体（tm结构体年份为"自1900年以来的年数"，月份为0-11）
    tmTime.tm_year = nYear - 1900;  // 年份转换：2023 → 2023-1900=123
    tmTime.tm_mon = nMonth - 1;     // 月份转换：9 → 8（9月为第8个索引）
    tmTime.tm_mday = nDay;         // 日期（1-31）
    tmTime.tm_hour = nHour;        // 小时（0-23）
    tmTime.tm_min = nMinute;       // 分钟（0-59）
    tmTime.tm_sec = (int)dSecond;  // 秒数（取整数部分，小数部分忽略）

    // mktime：将tm结构体转换为时间戳（考虑时区，此处默认UTC）
    return mktime(&tmTime);
}

// UTC时间转GPS时间函数
// 功能：将UTC时间字符串转换为GPS时间（相对于GPS时0点的秒数），包含闰秒补偿
// 参数：strUTC - 待转换的UTC时间字符串
// 返回：成功返回GPS时间秒数，失败返回-1
double CSatelliteCoordCalcDlg::UTC2GPST(const CString& strUTC)
{
    // 第一步：将UTC字符串转换为时间戳
    time_t utcTime = StringToTime(strUTC);
    if (utcTime == -1)
    {
        // 转换失败，提示用户时间格式错误
        AfxMessageBox(_T("时间格式错误！请使用YYYY-MM-DD HH:MM:SS"));
        return -1;
    }

    // 第二步：GPS时间比UTC时间快18秒（截至2023年的闰秒值）
    // UTC_TO_GPST_OFFSET：宏定义，值为18.0
    return (double)utcTime + UTC_TO_GPST_OFFSET;
}

// 计算GPS星期函数
// 功能：根据UTC年月日计算GPS星期数（GPS时间从1980-01-06 00:00:00开始）
// 参数：year/month/day - UTC年月日
// 返回：GPS星期数（从0开始计数）
int CSatelliteCoordCalcDlg::GetGPSWeek(int year, int month, int day)
{
    // 定义GPS起始时间 
    // GPS时间起点：1980年1月6日 00:00:00（UTC）
    struct tm gpsStart = { 0 };
    gpsStart.tm_year = 1980 - 1900;  // 年份：1980-1900=80
    gpsStart.tm_mon = 0;           // 月份：0（1月）
    gpsStart.tm_mday = 6;          // 日期：6
    gpsStart.tm_hour = 0;          // 小时：0
    gpsStart.tm_min = 0;           // 分钟：0
    gpsStart.tm_sec = 0;           // 秒数：0
    // 转换为时间戳（自1970-01-01以来的秒数）
    time_t gpsStartSec = mktime(&gpsStart);

    //  定义输入UTC时间 
    struct tm utcTime = { 0 };
    utcTime.tm_year = year - 1900;  // 输入年份转换
    utcTime.tm_mon = month - 1;     // 输入月份转换
    utcTime.tm_mday = day;         // 输入日期
    utcTime.tm_hour = 0;          // 小时：0（仅计算日期差，忽略时间）
    utcTime.tm_min = 0;           // 分钟：0
    utcTime.tm_sec = 0;           // 秒数：0
    // 转换为时间戳
    time_t utcSec = mktime(&utcTime);

    // 计算GPS星期数 
    // GPS星期数 = （输入时间戳 - GPS起始时间戳） / 每周秒数（604800 = 7*24*60*60）
    return (int)((utcSec - gpsStartSec) / 604800);
}

// UTC时间转GPS时秒数函数
// 功能：将UTC年月日时分秒转换为GPS时间（GPS星期内的秒数 + 星期数×每周秒数）
// 参数：year/month/day/hour/minute/second - UTC时间字段
// 返回：GPS时间秒数（相对于GPS起始时间的总秒数）
double CSatelliteCoordCalcDlg::UTC2GPSTSeconds(int year, int month, int day, int hour, int minute, double second)
{
    // 第一步：计算UTC时间的总秒数（时间戳）
    struct tm utcTime = { 0 };
    utcTime.tm_year = year - 1900;  // 年份转换
    utcTime.tm_mon = month - 1;     // 月份转换
    utcTime.tm_mday = day;         // 日期
    utcTime.tm_hour = hour;        // 小时
    utcTime.tm_min = minute;       // 分钟
    utcTime.tm_sec = (int)second;  // 秒数（取整数部分）
    time_t utcSec = mktime(&utcTime);

    // 第二步：转换为GPS时间（加闰秒18秒）
    double gpstSec = (double)utcSec + UTC_TO_GPST_OFFSET;

    // 第三步：返回GPS时间总秒数（无需拆分星期内秒数，直接用于后续计算）
    return gpstSec;
}

// GPS卫星坐标计算函数
// 功能：根据广播星历和UTC时间，计算卫星的地心地固坐标（X/Y/Z）
// 参数：
//   eph     广播星历结构体（包含卫星轨道参数）
//   strUTC  UTC时间字符串（计算时刻）
//   result  输出参数，存储计算后的卫星坐标
BOOL CSatelliteCoordCalcDlg::CalcGPSSatCoord(const SBroadcastEphemeris& eph, const CString& strUTC, SSatCoordResult& result)
{
    // 步骤1：校验关键星历参数有效性 
    // 关键参数：轨道长半径平方根（sqrt_a）、偏心率（e）、平近点角（M0）
    if (eph.sqrt_a <= 0 || eph.e < 0 || eph.e >= 1 || _isnan(eph.M0)) {
        CString msg;
        msg.Format(_T("星历参数异常！sqrt_a=%.3f, e=%.6f, M0=%.3f"),
            eph.sqrt_a, eph.e, eph.M0);
        AfxMessageBox(msg);  // 弹出参数异常提示
        return FALSE;        // 计算失败
    }

    // 步骤2：计算平均角速度n
    double a = eph.sqrt_a * eph.sqrt_a;  // 轨道长半径（m）= 长半径平方根的平方
    double n0 = sqrt(MU_GPS / (a * a * a));  // 无摄动平均角速度（rad/s）
    double n = n0 + eph.delta_n;  // 考虑摄动的平均角速度（rad/s）= 无摄动角速度 + 改正项

    // 步骤3：计算归化时间t_k 
    // 将UTC时间转换为GPS时间（秒数）
    double t_GPST = UTC2GPST(strUTC);
    if (t_GPST < 0)  // 时间转换失败
        return FALSE;

    // 归化时间：计算时刻与星历参考历元的差值（s）
    double t_k = t_GPST - eph.t_oe;
    // 归化时间范围限制：±3.5天（超过则加减一周，确保在一个星历周期内）
    if (t_k > 302400.0) t_k -= 604800.0;  // 302400s = 3.5天，604800s = 7天
    if (t_k < -302400.0) t_k += 604800.0;

    // 步骤4：计算平近点角M_k 
    // 平近点角：参考历元平近点角 + 平均角速度×归化时间（rad）
    double M_k = eph.M0 + n * t_k;
    if (_isnan(M_k)) {  // 平近点角为NaN（无效值）
        AfxMessageBox(_T("平近点角M_k为NaN！"));
        return FALSE;
    }

    //  步骤5：迭代求解偏近点角E_k 
    // 初始值：当偏心率e较大时（e>0.5），使用优化初始值（避免迭代收敛慢）
    double E_k = M_k;
    if (eph.e > 0.5) {
        E_k = (eph.e > 0) ? M_k / (1 - eph.e) : M_k;  // e=0时直接使用M_k
    }

    double E_prev = 0.0;          // 上一次迭代的E_k值
    int iterCount = 0;            // 迭代次数
    const double EPSILON = 1e-12; // 收敛阈值（迭代精度）
    const int MAX_ITER = 100;     // 最大迭代次数（防止死循环）

    // 牛顿迭代法求解开普勒方程：E = M + e×sin(E)
    do {
        E_prev = E_k;                          // 保存上一次迭代值
        E_k = M_k + eph.e * sin(E_k);          // 迭代公式
        iterCount++;                           // 迭代次数加1
    } while (fabs(E_k - E_prev) > EPSILON && iterCount < MAX_ITER);  // 未收敛且未超最大次数

    // 校验迭代结果：是否收敛
    if (_isnan(E_k) || iterCount >= MAX_ITER) {
        // 迭代失败，仅输出调试信息（不弹窗，避免连续干扰）
        CString msg;
        msg.Format(_T("卫星%s：E_k迭代失败（迭代次数：%d）"), eph.strPRN, iterCount);
        OutputDebugString(msg + _T("\n"));  // 调试输出到输出窗口
        return FALSE;
    }

    // 步骤6：计算真近点角V_k 
    double denominator = 1 - eph.e * cos(E_k);  // 分母（避免除以零）
    if (fabs(denominator) < 1e-10) {  // 分母接近0（异常情况）
        AfxMessageBox(_T("计算cosV时分母接近0！"));
        return FALSE;
    }

    // 真近点角的余弦和正弦值（rad）
    double cosV = (cos(E_k) - eph.e) / denominator;
    double sinV = (sqrt(1 - eph.e * eph.e) * sin(E_k)) / denominator;
    double V_k = atan2(sinV, cosV);  // 真近点角（rad），atan2确保象限正确

    // 步骤7：计算升交距角Φ_k 
    // 升交距角 = 真近点角 + 近地点角距（rad）
    double Phi_k = V_k + eph.omega_p;

    // 步骤8：计算轨道摄动改正项 
    double twoPhi = 2 * Phi_k;  // 2×升交距角（用于调和项计算）
    double delta_u = eph.C_uc * cos(twoPhi) + eph.C_us * sin(twoPhi);  // 升交距角改正（rad）
    double delta_r = eph.C_rc * cos(twoPhi) + eph.C_rs * sin(twoPhi);  // 卫星矢径改正（m）
    double delta_i = eph.C_ic * cos(twoPhi) + eph.C_is * sin(twoPhi);  // 轨道倾角改正（rad）

    // 步骤9：计算摄动后轨道参数 
    double u_k = Phi_k + delta_u;  // 摄动后升交距角（rad）
    double r_k = a * (1 - eph.e * cos(E_k)) + delta_r;  // 摄动后卫星矢径（m）
    double i_k = eph.i0 + delta_i + eph.i_dot * t_k;  // 摄动后轨道倾角（rad）

    // 步骤10：计算轨道平面坐标系坐标 
    double x_k = r_k * cos(u_k);  // 轨道平面X坐标（m）
    double y_k = r_k * sin(u_k);  // 轨道平面Y坐标（m）

    // 步骤11：计算升交点大地经度 
    // 升交点赤经变化：考虑地球自转（OMEGA_E为地球自转角速度）
    double Omega_k = eph.Omega0 + (eph.Omega_dot - OMEGA_E) * t_k - OMEGA_E * eph.t_oe;

    // 步骤12：转换为地心地固坐标系（WGS-84）
    double cosOmega = cos(Omega_k);  // 升交点赤经的余弦
    double sinOmega = sin(Omega_k);  // 升交点赤经的正弦
    double cosI = cos(i_k);          // 轨道倾角的余弦
    double sinI = sin(i_k);          // 轨道倾角的正弦

    // 地心地固坐标计算公式
    result.X = x_k * cosOmega - y_k * sinOmega * cosI;  // X坐标（m）
    result.Y = x_k * sinOmega + y_k * cosOmega * cosI;  // Y坐标（m）
    result.Z = y_k * sinI;                             // Z坐标（m）
    result.strPRN = eph.strPRN;                         // 卫星PRN号
    result.strUTC = strUTC;                             // 计算时刻（UTC）

    // 计算成功
    return TRUE;
}

// 字符串分割函数
// 功能：按指定分隔符分割字符串，处理多空格、制表符，支持科学计数法负号处理
// 参数：
//   str - 待分割的原始字符串
//   arr - 输出参数，存储分割后的字段
//   sep - 分隔符（默认空格）
void CSatelliteCoordCalcDlg::SplitString(const CString& str, CStringArray& arr, const CString& sep)
{
    arr.RemoveAll();  // 清空输出数组（避免残留旧数据）
    CString strTemp = str;  // 复制原始字符串（避免修改原数据）

    //  预处理1：统一分隔符 
    strTemp.Replace(_T('\t'), _T(' '));  // 将制表符（\t）替换为空格
    // 将连续空格（如"  "）替换为单个空格，避免空字段
    while (strTemp.Find(_T("  ")) != -1) {
        strTemp.Replace(_T("  "), _T(" "));
    }

    // 预处理2：处理科学计数法负号 
    // 场景：如"2.86e+02-8.32e-02" → 应分割为"2.86e+02"和"-8.32e-02"
    for (int i = 1; i < strTemp.GetLength(); i++) {
        // 条件：当前字符是负号，且前一个字符不是空格、e、E（说明是新字段的负号）
        if (strTemp[i] == _T('-') && strTemp[i - 1] != _T(' ') &&
            strTemp[i - 1] != _T('e') && strTemp[i - 1] != _T('E')) {
            strTemp.Insert(i, _T(' '));  // 在负号前插入空格
            i++;  // 跳过新增的空格，避免重复处理
        }
    }

    // 分割字段 
    int nStart = 0;                // 当前字段的起始索引
    int nLen = strTemp.GetLength(); // 字符串总长度

    while (nStart < nLen) {
        // 跳过分隔符（字符串开头的空格）
        while (nStart < nLen && strTemp.Find(sep, nStart) == nStart) {
            nStart += sep.GetLength();  // 移动到下一个非分隔符位置
        }
        if (nStart >= nLen) break;  // 已到字符串末尾，退出循环

        // 查找下一个分隔符的位置
        int nPos = strTemp.Find(sep, nStart);
        if (nPos == -1) nPos = nLen;  // 未找到分隔符，取字符串末尾

        // 提取当前字段（从nStart到nPos）
        CString strPart = strTemp.Mid(nStart, nPos - nStart);
        if (!strPart.IsEmpty()) {  // 跳过空字段
            arr.Add(strPart);      // 添加到输出数组
        }

        // 移动到下一个字段的起始位置
        nStart = nPos + sep.GetLength();
    }
}

// 打开星历文件按钮点击事件处理函数
// 功能：弹出文件选择框，加载RINEX广播星历文件，解析星历参数并存储
void CSatelliteCoordCalcDlg::OnBnClickedBtnOpen()
{
    //  弹出文件选择框 
    CFileDialog dlgOpen(
        TRUE,                          // 打开文件（而非保存）
        _T("rnx"),                      // 默认文件扩展名
        NULL,                           // 默认文件名
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,  // 隐藏"只读"选项 + 覆盖提示
        _T("RINEX广播星历文件 (*.rnx;*.obs)|*.rnx;*.obs|所有文件 (*.*)|*.*||")  // 文件过滤器
    );
    if (dlgOpen.DoModal() != IDOK) return;  // 用户取消选择，返回

    // 打开选中的星历文件
    CString strFilePath = dlgOpen.GetPathName();  // 获取选中文件的路径
    CStdioFile fileRinex;  // 文本文件对象（用于读取RINEX文件）

    // 以只读、文本模式打开文件
    if (!fileRinex.Open(strFilePath, CFile::modeRead | CFile::typeText))
    {
        AfxMessageBox(_T("文件打开失败！请检查路径或权限。"));
        return;
    }

    // 初始化解析变量 
    CString strLine;                // 存储当前读取的行
    BOOL bHeaderEnd = FALSE;         // 标记是否已跳过文件头部
    m_arrEphemeris.RemoveAll();      // 清空星历数组（避免残留旧数据）
    SBroadcastEphemeris currentEph;  // 存储当前解析的卫星星历
    BOOL bParsingEph = FALSE;        // 标记是否正在解析某颗卫星的星历
    int nLineIdx = 0;                // 星历参数行索引（0-6，共7行参数）
    // 星历参考时间字段（用于解析PRN行的时间）
    int nSatYear, nSatMonth, nSatDay, nSatHour, nSatMinute;
    double dSatSecond;

    // 逐行读取并解析文件 
    while (fileRinex.ReadString(strLine)) {
        strLine.Trim();  // 去除行首尾的空格和换行符

        // 跳过空行和注释行 
        if (strLine.IsEmpty() || strLine.Left(1) == _T("#")) {
            // 若正在解析某颗卫星的星历，遇到空行/注释行则中断解析
            if (bParsingEph) {
                bParsingEph = FALSE;  // 重置解析状态
                nLineIdx = 0;         // 重置行索引
            }
            continue;
        }

        // 跳过文件头部 
        if (!bHeaderEnd) {
            CString strLower = strLine;
            strLower.MakeLower();  // 转换为小写，避免大小写敏感
            // 查找"end of header"标记（RINEX文件头部结束标志）
            if (strLower.Find(_T("end of header")) != -1) {
                bHeaderEnd = TRUE;  // 标记为已跳过头部
            }
            continue;
        }

        // 分割当前行的字段
        CStringArray arrParts;
        SplitString(strLine, arrParts, _T(" "));  // 按空格分割字段
        if (arrParts.GetSize() == 0) {  // 分割失败（无有效字段）
            if (bParsingEph) {  // 正在解析星历，重置状态
                bParsingEph = FALSE;
                nLineIdx = 0;
            }
            continue;
        }

        // 检测卫星PRN行（开始解析新卫星）
        // PRN行特征：首字符为卫星系统标识（G/GPS、C/北斗、R/GLONASS等）
        CString strFirstChar = arrParts[0].Left(1);
        if (strFirstChar == _T("G") || strFirstChar == _T("S") || strFirstChar == _T("C") ||
            strFirstChar == _T("E") || strFirstChar == _T("R") || strFirstChar == _T("I")) {

            // 重置上一颗卫星的解析状态（若未解析完成）
            bParsingEph = FALSE;
            nLineIdx = 0;

            // 初始化当前卫星的星历结构体
            currentEph = SBroadcastEphemeris();
            currentEph.strPRN = arrParts[0];  // 存储卫星PRN号（如G01）
            bParsingEph = TRUE;              // 标记为正在解析星历
            nLineIdx = 0;                     // 重置行索引（开始解析第0行参数）

            // 解析PRN行的参考时间和钟差参数
            // PRN行格式（示例）：G01 2023 09 09 00 00 0.0 1.234e-05 5.67e-12 0.0
            if (arrParts.GetSize() >= 10) {
                nSatYear = _ttoi(arrParts[1]);          // 星历参考年
                nSatMonth = _ttoi(arrParts[2]);         // 星历参考月
                nSatDay = _ttoi(arrParts[3]);           // 星历参考日
                nSatHour = _ttoi(arrParts[4]);          // 星历参考时
                nSatMinute = _ttoi(arrParts[5]);        // 星历参考分
                dSatSecond = StrToDoubleWithD(arrParts[6]);  // 星历参考秒
                currentEph.a0 = StrToDoubleWithD(arrParts[7]);   // 卫星钟差（s）
                currentEph.a1 = StrToDoubleWithD(arrParts[8]);   // 卫星钟速（s/s）
                currentEph.a2 = StrToDoubleWithD(arrParts[9]);   // 卫星钟漂（s/s²）
                // 计算星历参考历元的GPS时间（秒数）
                currentEph.t_oe = UTC2GPSTSeconds(nSatYear, nSatMonth, nSatDay, nSatHour, nSatMinute, dSatSecond);
            }
            continue;  // 继续读取下一行（星历参数行）
        }

        // 解析卫星星历参数行（共7行）
        if (bParsingEph && nLineIdx >= 0 && nLineIdx <= 6) {
            switch (nLineIdx) {
            case 0:  // 第1行参数：IODE（星历数据龄期）、C_rc（矢径余弦调和项）、delta_n（角速度改正）、M0（平近点角）
                if (arrParts.GetSize() >= 4) {
                    currentEph.IODE = StrToDoubleWithD(arrParts[0]);
                    currentEph.C_rc = StrToDoubleWithD(arrParts[1]);
                    currentEph.delta_n = StrToDoubleWithD(arrParts[2]);
                    currentEph.M0 = StrToDoubleWithD(arrParts[3]);
                }
                break;
            case 1:  // 第2行参数：C_uc（升交距角余弦调和项）、e（偏心率）、C_us（升交距角正弦调和项）、sqrt_a（长半径平方根）
                if (arrParts.GetSize() >= 4) {
                    currentEph.C_uc = StrToDoubleWithD(arrParts[0]);
                    currentEph.e = StrToDoubleWithD(arrParts[1]);
                    currentEph.C_us = StrToDoubleWithD(arrParts[2]);
                    currentEph.sqrt_a = StrToDoubleWithD(arrParts[3]);
                }
                break;
            case 2:  // 第3行参数：C_ic（倾角余弦调和项）、Omega0（升交点赤经）、C_is（倾角正弦调和项）、i0（轨道倾角）
                if (arrParts.GetSize() >= 4) {
                    currentEph.C_ic = StrToDoubleWithD(arrParts[0]);
                    currentEph.Omega0 = StrToDoubleWithD(arrParts[1]);
                    currentEph.C_is = StrToDoubleWithD(arrParts[2]);
                    currentEph.i0 = StrToDoubleWithD(arrParts[3]);
                }
                break;
            case 3:  // 第4行参数：omega_p（近地点角距）、C_rc（冗余）、Omega_dot（升交点赤经变化率）、i_dot（倾角变化率）
                if (arrParts.GetSize() >= 4) {
                    currentEph.omega_p = StrToDoubleWithD(arrParts[0]);
                    currentEph.Omega_dot = StrToDoubleWithD(arrParts[2]);
                    currentEph.i_dot = StrToDoubleWithD(arrParts[3]);
                }
                break;
            case 4:  // 第5行：冗余参数（如健康状态、备用字段），无需解析
            case 5:  // 第6行：冗余参数
            case 6:  // 第7行：冗余参数
                break;
            }
            nLineIdx++;  // 移动到下一行参数

            // 星历参数解析完成（7行） 
            if (nLineIdx == 7) {
                // 校验关键参数有效性（避免无效星历）
                if (currentEph.sqrt_a <= 0 || currentEph.e < 0 || currentEph.e >= 1 || _isnan(currentEph.M0)) {
                    CString msg;
                    msg.Format(_T("卫星%s参数无效：sqrt_a=%.3f, e=%.6f"),
                        currentEph.strPRN, currentEph.sqrt_a, currentEph.e);
                    OutputDebugString(msg + _T("\n"));  // 调试输出
                    bParsingEph = FALSE;
                    nLineIdx = 0;
                    continue;  // 跳过无效星历
                }

                // 解析成功，添加到星历数组
                m_arrEphemeris.Add(currentEph);
                m_bAnyEphemerisLoaded = TRUE;  // 标记有有效星历
                bParsingEph = FALSE;           // 重置解析状态
                nLineIdx = 0;                  // 重置行索引
            }
        }
    }

    // 关闭文件并提示结果 
    fileRinex.Close();

    // 根据星历数组大小判断加载结果
    if (m_arrEphemeris.GetSize() > 0) {
        AfxMessageBox(_T("星历加载成功！已识别有效卫星数据。"));
        m_bEphemerisLoaded = TRUE;  // 标记星历加载成功
    }
    else {
        AfxMessageBox(_T("星历加载失败！未找到有效卫星数据。"));
        m_bEphemerisLoaded = FALSE;  // 标记星历加载失败
    }
}

// 计算所有卫星坐标按钮点击事件处理函数
// 功能：遍历所有加载的星历，计算指定UTC时间的卫星坐标，显示结果并记录失败卫星
void CSatelliteCoordCalcDlg::OnBnClickedBtnCalc()
{
    // 输入验证
    UpdateData(TRUE);  // 读取编辑框内容到成员变量
    if (!m_bAnyEphemerisLoaded) {  // 无有效星历，提示用户加载
        AfxMessageBox(_T("请先加载有效广播星历！"));
        return;
    }

    // 初始化计算变量 
    CString strUTC = CombineTimeString();  // 组合UTC时间字符串
    m_listResult.DeleteAllItems();         // 清空结果列表（避免残留旧数据）
    CStringArray arrSuccessPRN;            // 记录计算成功的卫星PRN（去重）
    m_arrAllSatResults.RemoveAll();        // 清空卫星坐标数组（用于后续绘图）

    //  遍历星历，计算每颗卫星的坐标 
    for (int i = 0; i < m_arrEphemeris.GetSize(); i++) {
        SBroadcastEphemeris eph = m_arrEphemeris[i];  // 当前卫星的星历
        SSatCoordResult result;                       // 存储计算结果

        // 计算卫星坐标
        if (CalcGPSSatCoord(eph, strUTC, result)) {
            // 计算成功：添加到结果列表 
            // 插入新行，PRN号作为第一列
            int nRow = m_listResult.InsertItem(m_listResult.GetItemCount(), result.strPRN);
            CString strX, strY, strZ;
            // 格式化坐标值（保留3位小数）
            strX.Format(_T("%.3f"), result.X);
            strY.Format(_T("%.3f"), result.Y);
            strZ.Format(_T("%.3f"), result.Z);
            // 设置列表列内容：X/Y/Z坐标
            m_listResult.SetItemText(nRow, 1, strX);
            m_listResult.SetItemText(nRow, 2, strY);
            m_listResult.SetItemText(nRow, 3, strZ);

            // 添加到坐标数组（供后续绘制卫星图）
            m_arrAllSatResults.Add(result);

            // 记录成功的PRN（去重，避免重复记录同一卫星）
            BOOL bExists = FALSE;
            for (int j = 0; j < arrSuccessPRN.GetSize(); j++) {
                if (arrSuccessPRN[j] == result.strPRN) {
                    bExists = TRUE;
                    break;
                }
            }
            if (!bExists) {
                arrSuccessPRN.Add(result.strPRN);
            }
        }
        // 计算失败：暂不处理，后续统一汇总
    }

    // 汇总计算结果（成功/失败）
    // 收集所有星历中的唯一PRN（去重）
    CStringArray arrAllPRN;
    for (int i = 0; i < m_arrEphemeris.GetSize(); i++) {
        CString prn = m_arrEphemeris[i].strPRN;
        BOOL exists = FALSE;
        for (int j = 0; j < arrAllPRN.GetSize(); j++) {
            if (arrAllPRN[j] == prn) {
                exists = TRUE;
                break;
            }
        }
        if (!exists) {
            arrAllPRN.Add(prn);
        }
    }

    // 筛选计算失败的卫星PRN
    CString strError汇总;
    for (int i = 0; i < arrAllPRN.GetSize(); i++) {
        CString prn = arrAllPRN[i];
        BOOL bSuccess = FALSE;
        // 检查该PRN是否在成功列表中
        for (int j = 0; j < arrSuccessPRN.GetSize(); j++) {
            if (arrSuccessPRN[j] == prn) {
                bSuccess = TRUE;
                break;
            }
        }
        if (!bSuccess) {
            strError汇总 += prn + _T("、");  // 拼接失败PRN
        }
    }

    //  显示计算结果提示 
    if (!strError汇总.IsEmpty()) {
        // 去除末尾的"、"，提示失败卫星
        strError汇总 = _T("以下卫星计算失败：") + strError汇总.Left(strError汇总.GetLength() - 1);
        AfxMessageBox(strError汇总);
    }
    else {
        // 所有卫星计算成功
        AfxMessageBox(_T("所有卫星坐标计算完成！"));
    }
}

// 计算单个卫星坐标按钮点击事件处理函数
// 功能：根据用户输入的PRN，查找匹配的星历，计算并显示该卫星的坐标
void CSatelliteCoordCalcDlg::OnBnClickedBtnCalcSingle()
{
    // 输入验证
    UpdateData(TRUE);  // 读取编辑框内容到成员变量

    // 验证1：PRN输入不为空
    if (m_strSinglePRN.IsEmpty()) {
        AfxMessageBox(_T("请输入卫星PRN（如G01、C05）！"));
        return;
    }

    // 验证2：PRN格式合法（至少2个字符，如G01）
    if (m_strSinglePRN.GetLength() < 2) {
        AfxMessageBox(_T("PRN格式错误（示例：G01）！"));
        return;
    }

    // 验证3：PRN首字符为合法系统标识（G/C/R/E/S/I）
    char cType = toupper(m_strSinglePRN[0]);
    if (cType != 'G' && cType != 'C' && cType != 'R' && cType != 'E' && cType != 'S' && cType != 'I') {
        AfxMessageBox(_T("PRN格式错误，首字符应为G/C/R/E/S/I！"));
        return;
    }

    // 验证4：已加载有效星历
    if (!m_bAnyEphemerisLoaded) {
        AfxMessageBox(_T("请先加载有效广播星历！"));
        return;
    }

    //  初始化计算变量 
    CString strUTC = CombineTimeString();  // 组合UTC时间字符串
    int nMatchCount = 0;                  // 记录匹配的星历数量
    SSatCoordResult result;               // 存储计算结果

    // 清空现有结果（列表和坐标数组）
    m_listResult.DeleteAllItems();
    m_arrAllSatResults.RemoveAll();

    // 查找匹配PRN的星历并计算
    for (int i = 0; i < m_arrEphemeris.GetSize(); i++) {
        SBroadcastEphemeris eph = m_arrEphemeris[i];
        // 不区分大小写匹配PRN（如G01和g01都匹配）
        if (eph.strPRN.CompareNoCase(m_strSinglePRN) == 0) {
            // 计算该卫星的坐标
            if (CalcGPSSatCoord(eph, strUTC, result)) {
                // 计算成功：添加到结果列表
                int nRow = m_listResult.InsertItem(m_listResult.GetItemCount(), result.strPRN);
                CString strX, strY, strZ;
                strX.Format(_T("%.3f"), result.X);
                strY.Format(_T("%.3f"), result.Y);
                strZ.Format(_T("%.3f"), result.Z);
                m_listResult.SetItemText(nRow, 1, strX);
                m_listResult.SetItemText(nRow, 2, strY);
                m_listResult.SetItemText(nRow, 3, strZ);

                // 添加到坐标数组（供后续绘图）
                m_arrAllSatResults.Add(result);
                nMatchCount++;  // 匹配计数加1
            }
        }
    }

    // 显示计算结果反馈 
    if (nMatchCount == 0) {
        // 未找到匹配的星历
        AfxMessageBox(_T("未找到指定PRN的星历数据！"));
    }
    else {
        // 找到匹配星历并计算完成
        CString strMsg;
        strMsg.Format(_T("找到 %d 条匹配的星历数据，计算完成！"), nMatchCount);
        AfxMessageBox(strMsg);
    }
}

// 保存计算结果按钮点击事件处理函数
// 功能：弹出保存文件对话框，将结果列表中的卫星坐标保存为TXT或XLS文件
void CSatelliteCoordCalcDlg::OnBnClickedBtnSave()
{
    // 验证是否有计算结果 
    if (m_listResult.GetItemCount() == 0) {
        AfxMessageBox(_T("无计算结果可保存！"));
        return;
    }

    // 弹出保存文件对话框
    CFileDialog dlgSave(
        FALSE,                          // 保存文件（而非打开）
        _T("txt"),                      // 默认文件扩展名
        _T("SatCoordResult"),           // 默认文件名（如SatCoordResult.txt）
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,  // 隐藏“只读”选项 + 覆盖提示
        _T("文本文件 (*.txt)|*.txt|Excel文件 (*.xls)|*.xls|所有文件 (*.*)|*.*||")  // 文件过滤器
    );
    if (dlgSave.DoModal() != IDOK) return;  // 用户取消保存，返回

    //  打开保存文件 
    CString strSavePath = dlgSave.GetPathName();  // 获取保存路径
    CStdioFile fileSave;  // 文本文件对象（用于写入结果）

    // 以创建、写入、文本模式打开文件（若文件已存在则覆盖）
    if (!fileSave.Open(strSavePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) {
        AfxMessageBox(_T("文件创建失败！"));
        return;
    }

    // 写入文件内容 
    // 写入表头（列标题）
    CString strHeader = _T("卫星PRN\tX坐标（m）\tY坐标（m）\tZ坐标（m）\r\n");
    fileSave.WriteString(strHeader);  // 写入表头

    // 写入每条计算结果
    for (int i = 0; i < m_listResult.GetItemCount(); i++) {
        CString strLine;
        // 格式化每行内容：PRN + X + Y + Z（制表符分隔，Excel可识别）
        strLine.Format(_T("%s\t%s\t%s\t%s\r\n"),
            (LPCTSTR)m_listResult.GetItemText(i, 0),  // 卫星PRN
            (LPCTSTR)m_listResult.GetItemText(i, 1),  // X坐标
            (LPCTSTR)m_listResult.GetItemText(i, 2),  // Y坐标
            (LPCTSTR)m_listResult.GetItemText(i, 3)); // Z坐标
        fileSave.WriteString(strLine);  // 写入当前行
    }

    // 关闭文件并提示 
    fileSave.Close();
    // 提示用户保存成功及保存路径
    AfxMessageBox(_T("结果已保存至：") + strSavePath);
}

// 清除数据按钮点击事件处理函数
// 功能：清空计算结果列表、坐标数组，清除绘图区域，重置界面状态
void CSatelliteCoordCalcDlg::OnBnClickedBtnClear()
{
    //  1. 清空结果列表和坐标数组 
    m_listResult.DeleteAllItems();         // 清空结果列表（删除所有行）
    m_arrAllSatResults.RemoveAll();        // 清空卫星坐标数组（用于绘图）

    //  2. 清除绘图区域并重新绘制背景
    // 获取坐标系图片控件的窗口指针
    CWnd* pStatic = GetDlgItem(IDC_STATIC_COORDSYS);
    if (pStatic != nullptr)
    {
        // 获取控件的设备上下文（用于绘图）
        CDC* pDC = pStatic->GetDC();
        CRect rect;
        pStatic->GetClientRect(&rect);  // 获取控件客户区大小

        // 用对话框背景色填充整个控件区域（清除原有绘图）
        COLORREF bkColor = pDC->GetBkColor();  // 获取设备上下文的背景色（与对话框一致）
        CBrush brush(bkColor);                // 创建背景色画刷
        pDC->FillRect(rect, &brush);          // 填充控件区域

        // 重新绘制坐标系背景图片（恢复空白状态）
        if (m_pCoordsysImage != nullptr && m_pCoordsysImage->GetLastStatus() == Ok)
        {
            Graphics graphics(pDC->m_hDC);  // 创建GDI+绘图对象
            // 拉伸图片填充控件
            graphics.DrawImage(m_pCoordsysImage, 0, 0, rect.Width(), rect.Height());
        }

        // 释放设备上下文（避免资源泄漏）
        pStatic->ReleaseDC(pDC);
        pStatic->UpdateWindow();  // 立即刷新控件（确保界面更新）
    }

    //  重置编辑框显示 
    UpdateData(FALSE);  // 将成员变量值更新到编辑框（恢复默认值）

    //  提示用户清除完成 
    AfxMessageBox(_T("已清除所有计算结果和图像！"));
}

// 初始化卫星类型-颜色映射函数
// 功能：为不同卫星系统（GPS、北斗等）分配唯一颜色，用于绘图时区分
void CSatelliteCoordCalcDlg::InitSatelliteColors()
{
    // 格式：m_satTypeColors['系统标识'] = Color(透明度, R, G, B)
    m_satTypeColors['G'] = Color(255, 255, 0, 0);      // GPS (G) - 红色（透明度255=不透明）
    m_satTypeColors['C'] = Color(255, 0, 255, 0);      // 北斗 (C) - 绿色
    m_satTypeColors['R'] = Color(255, 0, 0, 255);      // GLONASS (R) - 蓝色
    m_satTypeColors['E'] = Color(255, 255, 255, 0);    // Galileo (E) - 黄色
    m_satTypeColors['S'] = Color(255, 255, 0, 255);    // 其他系统 (S) - 紫色
    m_satTypeColors['I'] = Color(255, 128, 0, 128);    // 印度IRNSS (I) - 紫红色
}

// 根据卫星PRN获取对应颜色函数
// 功能：通过PRN首字符判断卫星系统，返回预定义的颜色（未知系统返回灰色）
// 参数：prn - 卫星PRN号（如G01、C05）
// 返回：卫星系统对应的颜色
Color CSatelliteCoordCalcDlg::GetSatelliteColor(const CString& prn)
{
    // PRN为空时返回默认灰色
    if (prn.IsEmpty())
        return Color(255, 128, 128, 128);  // 默认灰色（R=128, G=128, B=128）

    // 提取PRN首字符（转换为大写，避免大小写敏感）
    char type = toupper(prn[0]);
    // 查找该系统对应的颜色，找到则返回，未找到则返回默认灰色
    if (m_satTypeColors.find(type) != m_satTypeColors.end())
        return m_satTypeColors[type];

    // 未知卫星系统返回灰色
    return Color(255, 128, 128, 128);
}

// 绘制卫星轨道图按钮点击事件处理函数
// 功能：根据计算的卫星坐标，在坐标系控件上绘制卫星点、PRN号和图例，实现可视化展示
void CSatelliteCoordCalcDlg::OnBnClickedBtnDraw()
{
    // 1. 校验绘图前提条件 
    // 条件1：已有卫星坐标计算结果（避免无数据绘图）
    if (m_arrAllSatResults.IsEmpty()) {
        AfxMessageBox(_T("请先点击「计算卫星坐标」按钮，获取卫星坐标后再绘制！"));
        return;
    }
    // 条件2：坐标系图片加载成功（避免无背景绘图）
    if (m_pCoordsysImage == nullptr || m_pCoordsysImage->GetLastStatus() != Ok) {
        AfxMessageBox(_T("坐标系图片未加载成功，无法绘制！"));
        return;
    }

    //  2. 初始化卫星类型-颜色映射 
    std::map<char, Color> satTypeColors = m_satTypeColors;  // 复制预定义的颜色映射
    Color defaultColor = Color(255, 128, 128, 128);        // 未知系统默认颜色（灰色）

    //  3. 获取绘图载体（控件）和初始化GDI+ 
    // 获取坐标系控件的设备上下文（用于绑定GDI+绘图对象）
    CDC* pDC = m_staticCoordsys.GetDC();
    CRect rectDraw;
    m_staticCoordsys.GetClientRect(&rectDraw);  // 获取控件客户区大小（绘图范围）
    Graphics graphics(pDC->m_hDC);             // 创建GDI+绘图对象，绑定到控件DC

    // 4. 绘制坐标系背景图片 
    // 拉伸背景图片，使其完全填充控件客户区
    graphics.DrawImage(m_pCoordsysImage,
        0, 0,                                  // 绘图起始坐标（控件左上角）
        rectDraw.Width(), rectDraw.Height());   // 绘图宽高（与控件一致）

    //  5. 计算卫星坐标到屏幕像素的缩放参数 
    // 统计所有卫星坐标的X/Z范围（Y坐标暂不用于2D绘图，仅用X/Z）
    double minX = 1e20, maxX = -1e20;  // X坐标最小值/最大值（初始值设为极大/极小）
    double minZ = 1e20, maxZ = -1e20;  // Z坐标最小值/最大值
    for (int i = 0; i < m_arrAllSatResults.GetSize(); i++) {
        SSatCoordResult sat = m_arrAllSatResults[i];
        minX = min(minX, sat.X);  // 更新X最小值
        maxX = max(maxX, sat.X);  // 更新X最大值
        minZ = min(minZ, sat.Z);  // 更新Z最小值
        maxZ = max(maxZ, sat.Z);  // 更新Z最大值
    }

    // 计算缩放比例（确保所有卫星点都能在控件内显示）
    double imgWidth = rectDraw.Width() * 0.8;   // 有效绘图宽度
    double imgHeight = rectDraw.Height() * 0.8; // 有效绘图高度
    double scaleX = imgWidth / (maxX - minX);    // X方向缩放比例（坐标差→像素差）
    double scaleZ = imgHeight / (maxZ - minZ);   // Z方向缩放比例
    double scale = min(scaleX, scaleZ);          // 取较小缩放比例（避免拉伸变形）
    double offsetX = rectDraw.Width() * 0.1;    // X方向偏移（居中显示）
    double offsetY = rectDraw.Height() * 0.1;   // Y方向偏移

    // 坐标转换lambda表达式（卫星坐标→屏幕像素坐标）
    auto toPixelX = [&](double x) { return (x - minX) * scale + offsetX; };  // X坐标转换
    auto toPixelY = [&](double z) { return (maxZ - z) * scale + offsetY; };  // Z坐标转换（屏幕Y向下，需反转）

    //  6. 绘制所有卫星点和PRN号 
    // 初始化PRN号字体
    Gdiplus::Font fontPRN(L"宋体", 8);
    // 初始化PRN号文字画刷
    SolidBrush brushPRNText(Color(0, 0, 0));

    // 遍历所有卫星坐标，逐个绘制
    for (int i = 0; i < m_arrAllSatResults.GetSize(); i++) {
        SSatCoordResult sat = m_arrAllSatResults[i];
        if (sat.strPRN.IsEmpty()) continue;  // PRN为空则跳过（无效数据）

        // 确定当前卫星的颜色
        char type = toupper(sat.strPRN[0]);  // 提取卫星系统标识
        Color satColor = defaultColor;
        if (satTypeColors.find(type) != satTypeColors.end()) {
            satColor = satTypeColors[type];  // 使用预定义颜色
        }
        Gdiplus::SolidBrush brushSat(satColor);  // 创建卫星点画刷

        // 转换卫星坐标为屏幕像素坐标
        float pixelX = (float)toPixelX(sat.X);
        float pixelY = (float)toPixelY(sat.Z);

        // 绘制卫星点
        graphics.FillEllipse(&brushSat,
            pixelX - 5.0f,  // 圆形左上角X
            pixelY - 5.0f,  // 圆形左上角Y
            10.0f, 10.0f);  // 圆形宽高

        // 绘制卫星PRN号
        CString strPRN = sat.strPRN;
        graphics.DrawString(
            strPRN.GetString(),               // PRN字符串
            strPRN.GetLength(),               // 字符串长度
            &fontPRN,                        // 字体
            Gdiplus::PointF(pixelX + 6, pixelY - 4),  // 文字位置
            &brushPRNText                     // 文字画刷
        );
    }

    //  7. 绘制图例（卫星类型-颜色对应表）
    DrawLegend(&graphics, rectDraw);

    // 8. 释放资源并提示 
    m_staticCoordsys.ReleaseDC(pDC);  // 释放控件的设备上下文（避免资源泄漏）
    AfxMessageBox(_T("所有卫星点已按类型着色并绘制图例！"));  // 提示绘图完成
}

// 绘制图例函数
// 功能：在坐标系控件右上角绘制卫星类型-颜色对应表
// 参数：
//   pGraphics GDI+绘图对象（已绑定到控件DC）
//   rect      控件客户区矩形（用于定位图例）
void CSatelliteCoordCalcDlg::DrawLegend(Graphics* pGraphics, CRect& rect)
{
    // 图例绘制前提：颜色映射非空且绘图对象有效
    if (m_satTypeColors.empty() || !pGraphics) return;

    //  1. 设定图例布局参数 
    int legendX = rect.right - 130;  // 图例左上角X（控件右边缘左移）
    int legendY = 20;                // 图例左上角Y（控件上边缘下移）
    int itemHeight = 30;             // 每个图例项高度（颜色方块+文字）
    int boxSize = 12;                // 颜色方块尺寸
    int legendWidth = 130;           // 图例总宽度
    // 图例总高度 = 图例项数量×项高度 + 20px（标题和边距）
    int legendHeight = m_satTypeColors.size() * itemHeight + 20;

    // 绘制图例背景和边框 
    // 图例背景画刷（浅灰色，半透明，R，G，B，透明度）
    SolidBrush bgBrush(Color(240, 240, 240, 240));
    // 图例边框画笔（灰色，线宽1px）
    Pen borderPen(Color(180, 180, 180), 1);
    // 绘制背景矩形
    pGraphics->FillRectangle(&bgBrush,
        (REAL)(legendX - 10), (REAL)(legendY - 10),  // 背景左上角
        (REAL)legendWidth, (REAL)legendHeight);     // 背景宽高
    // 绘制边框
    pGraphics->DrawRectangle(&borderPen,
        (REAL)(legendX - 10), (REAL)(legendY - 10),
        (REAL)legendWidth, (REAL)legendHeight);

    // 绘制图例标题（"卫星类型"） 
    Gdiplus::Font titleFont(L"Arial", 8, FontStyleBold);  // 标题字体
    SolidBrush textBrush(Color::Black);                   // 标题文字画刷
    pGraphics->DrawString(L"卫星类型", -1, &titleFont,
        PointF((REAL)legendX, (REAL)legendY),  // 标题位置
        &textBrush);
    legendY += 22;  // 标题下方偏移15px，预留空间给图例项

    // 绘制每个图例项（颜色方块+系统说明） 
    Gdiplus::Font font(L"Arial", 7);  // 图例项文字字体
    int index = 0;                   // 图例项索引（从0开始）

    // 遍历颜色映射，逐个绘制图例项
    for (auto& pair : m_satTypeColors)
    {
        // 绘制颜色方块
        SolidBrush colorBrush(pair.second);  // 使用当前系统的颜色
        pGraphics->FillRectangle(&colorBrush,
            (REAL)legendX,                          // 颜色方块X
            (REAL)(legendY + index * itemHeight),   // 颜色方块Y（按索引偏移）
            (REAL)boxSize, (REAL)boxSize);          // 颜色方块尺寸

        // 绘制颜色方块边框
        pGraphics->DrawRectangle(&borderPen,
            (REAL)legendX,
            (REAL)(legendY + index * itemHeight),
            (REAL)boxSize, (REAL)boxSize);

        // 设置图例项文字说明
        CString strDesc;
        switch (pair.first)  // 根据卫星系统标识设置说明文字
        {
        case 'G': strDesc = L"GPS (G)";     break;  // GPS系统
        case 'C': strDesc = L"北斗 (C)";    break;  // 北斗系统
        case 'R': strDesc = L"GLONASS (R)"; break;  // GLONASS系统
        case 'E': strDesc = L"Galileo (E)"; break;  // Galileo系统
        case 'S': strDesc = L"其他 (S)";    break;  // 其他系统
        case 'I': strDesc = L"IRNSS (I)";   break;  // 印度IRNSS系统
        default:  strDesc.Format(_T("%c 系统"), pair.first);  // 未知系统默认格式
        }

        // 绘制图例项文字
        pGraphics->DrawString(strDesc.GetString(), -1, &font,
            PointF((REAL)(legendX + boxSize + 5), (REAL)(legendY + index * itemHeight)),
            &textBrush);

        index++;  // 移动到下一个图例项
    }
}

// 鼠标左键双击事件处理函数
// 功能：双击坐标系控件时，打开放大视图对话框，展示更大尺寸的卫星图
void CSatelliteCoordCalcDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // 调用父类双击事件处理，确保基础功能正常
    CDialogEx::OnLButtonDblClk(nFlags, point);

    // 检查双击位置是否在坐标系控件内
    CWnd* pStatic = GetDlgItem(IDC_STATIC_COORDSYS);  // 获取坐标系控件指针
    if (pStatic)
    {
        CRect rect;
        pStatic->GetWindowRect(&rect);  // 获取控件屏幕坐标
        ScreenToClient(&rect);          // 转换为对话框客户区坐标
        // 条件：双击点在控件内，且已有卫星坐标结果
        if (rect.PtInRect(point) && !m_arrAllSatResults.IsEmpty())
        {
            CEnlargedViewDlg dlg;                  // 创建放大视图对话框对象
            dlg.SetData(m_pCoordsysImage, m_arrAllSatResults);  // 传递绘图数据（背景图+卫星坐标）
            dlg.DoModal();  // 以模态方式显示放大对话框（阻塞当前窗口，关闭后返回）
        }
    }
}

// 对话框销毁事件处理函数
// 功能：释放程序运行过程中分配的资源（图片、GDI+），避免内存泄漏
void CSatelliteCoordCalcDlg::OnDestroy()
{
    // 调用父类销毁函数，确保对话框基础资源释放
    CDialogEx::OnDestroy();

    // 释放坐标系图片资源 
    if (m_pCoordsysImage != nullptr)
    {
        delete m_pCoordsysImage;  // 释放图片对象
        m_pCoordsysImage = nullptr;  // 置空指针，避免野指针
    }

    // 关闭GDI+
    GdiplusShutdown(m_gdiplusToken);  // 使用初始化时的令牌关闭GDI+
}

// 结果列表项变化事件处理函数
// 功能：空实现，预留用于后续扩展（如选中列表项时高亮对应卫星点）
void CSatelliteCoordCalcDlg::OnLvnItemchangedListResult(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: 后续可添加列表项选中逻辑（如高亮对应卫星点）
    *pResult = 0;  // 返回0表示事件已处理
}

// 测站标题静态控件点击事件处理函数
// 功能：空实现，预留用于后续扩展（如显示测站信息弹窗）
void CSatelliteCoordCalcDlg::OnStnClickedStaticStationTitle()
{
    // TODO: 后续可添加测站信息展示逻辑
}

// 坐标系静态控件点击事件处理函数
// 功能：空实现，预留用于后续扩展（如点击卫星点显示详情）
void CSatelliteCoordCalcDlg::OnStnClickedStaticCoordsys()
{
    // TODO: 后续可添加卫星点点击交互逻辑
}

// 测站Z坐标静态控件点击事件处理函数
// 功能：空实现，预留用于后续扩展（如显示Z坐标说明）
void CSatelliteCoordCalcDlg::OnStnClickedStaticStationZ()
{
    // TODO: 后续可添加测站Z坐标说明逻辑
}