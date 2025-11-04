#pragma once
// 卫星坐标计算主对话框头文件

#include "afxwin.h"  
#include <gdiplus.h>  
#include <map>

using namespace Gdiplus;

// 地球引力常数（GPS），单位：m³/s²
#define MU_GPS 3.986005e14  
// 地球自转角速度，单位：rad/s
#define OMEGA_E 7.292115e-5 
// UTC到GPST的固定时差（截至2023年，闰秒18秒）
#define UTC_TO_GPST_OFFSET 18.0

// 卫星坐标计算结果结构体（地心地固坐标）
struct SSatCoordResult {
    CString strPRN;       // 卫星PRN编号
    CString strUTC;       // 计算时刻（UTC格式）
    double X = 0.0;       // X坐标（m，WGS-84坐标系）
    double Y = 0.0;       // Y坐标（m）
    double Z = 0.0;       // Z坐标（m）
};

// CSatelliteCoordCalcDlg 对话框类：实现卫星坐标计算与可视化功能
class CSatelliteCoordCalcDlg : public CDialogEx
{
public:
    // 标记是否有至少一颗卫星的星历加载成功
    BOOL m_bAnyEphemerisLoaded;

    // 标准构造函数
    // 参数：pParent - 父窗口指针，默认为nullptr
    CSatelliteCoordCalcDlg(CWnd* pParent = nullptr);

public:
    // UTC时间（年/月/日/时/分/秒）转GPS时秒数
    // 返回：相对于GPS星期的秒数
    double UTC2GPSTSeconds(int year, int month, int day, int hour, int minute, double second);

    // 计算GPS星期（GPS时间从1980-01-06 00:00:00开始，每周604800秒）
    int GetGPSWeek(int year, int month, int day);

    // 对话框资源ID（设计时使用）
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SATELLITECOORDCALC_DIALOG };
#endif

protected:
    ULONG_PTR m_gdiplusToken;  // GDI+初始化令牌
    Image* m_pCoordsysImage;   // 卫星坐标系背景图片对象
    // DDX/DDV 数据交换支持
    virtual void DoDataExchange(CDataExchange* pDX);

    // 实现部分
protected:
    HICON m_hIcon;  // 窗口图标

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();                     // 对话框初始化
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam); // 系统命令处理
    afx_msg void OnPaint();                          // 绘图事件处理
    afx_msg HCURSOR OnQueryDragIcon();               // 拖拽图标处理
    afx_msg void OnDestroy();                        // 对话框销毁处理
    DECLARE_MESSAGE_MAP()                            // 消息映射声明

        // 自定义结构体：广播星历参数（存储卫星轨道信息）
        struct SBroadcastEphemeris {
        CString strPRN;       // 卫星PRN号
        double t_oe = 0.0;          // 星历参考历元（秒）
        double M0 = 0.0;            // 参考历元平近点角（弧度）
        double delta_n = 0.0;       // 平均角速度改正（弧度/秒）
        double e = 0.0;             // 轨道偏心率
        double sqrt_a = 0.0;        // 轨道长半径平方根（米^(1/2)）
        double omega_p = 0.0;       // 近地点角距（弧度）
        double i0 = 0.0;            // 参考历元轨道倾角（弧度）
        double Omega0 = 0.0;        // 参考历元升交点赤经（弧度）
        double Omega_dot = 0.0;     // 升交点赤经变化率（弧度/秒）
        double i_dot = 0.0;         // 轨道倾角变化率（弧度/秒）
        double C_uc = 0.0;          // 升交距角余弦调和项振幅（弧度）
        double C_us = 0.0;          // 升交距角正弦调和项振幅（弧度）
        double C_rc = 0.0;          // 卫星矢径余弦调和项振幅（米）
        double C_rs = 0.0;          // 卫星矢径正弦调和项振幅（米）
        double C_ic = 0.0;          // 轨道倾角余弦调和项振幅（弧度）
        double C_is = 0.0;          // 轨道倾角正弦调和项振幅（弧度）
        double a0 = 0.0;            // 卫星钟差（秒）
        double a1 = 0.0;            // 卫星钟速（秒/秒）
        double a2 = 0.0;            // 卫星钟漂（秒/秒²）
        double IODE = 0.0;          // 星历数据龄期
    };

    // 控件关联变量与事件
public:
    afx_msg void OnEnChangeEditTime();       // 时间编辑框内容变化处理
    afx_msg void OnEnChangeTimeFields();     // 时间字段内容变化处理

    CListCtrl m_listResult; // 展示卫星坐标计算结果（X/Y/Z列）
    CStatic m_staticDraw;   // 轨道图绘制载体（IDC_STATIC_DRAW）
    CStatic m_staticCoordsys; // 坐标系图片载体（显示背景与卫星点）

private:
    // 卫星类型与颜色映射表（用于区分不同系统卫星）
    std::map<char, Color> m_satTypeColors;

    // 初始化卫星颜色映射（为不同系统卫星分配颜色）
    void InitSatelliteColors();

    // 根据卫星PRN获取对应的颜色
    Color GetSatelliteColor(const CString& prn);

    // 绘制图例（显示卫星类型与颜色的对应关系）
    // 参数：
    //   pGraphics - GDI+绘图对象指针
    //   rect      - 对话框客户区矩形（用于定位图例）
    void DrawLegend(Graphics* pGraphics, CRect& rect);

    // 时间字段变量
    int m_nYear;       // 年
    int m_nMonth;      // 月
    int m_nDay;        // 日
    int m_nHour;       // 时
    int m_nMinute;     // 分
    double m_dSecond;  // 秒（支持小数）

    // 组合年月日时分秒为UTC字符串（格式：YYYY-MM-DD HH:MM:SS.sss）
    CString CombineTimeString();

    // 单个卫星PRN输入变量
    CString m_strSinglePRN;

    // 测站坐标变量（地心地固坐标系）
    double m_dStationX;   // 测站X坐标（米）
    double m_dStationY;   // 测站Y坐标（米）
    double m_dStationZ;   // 测站Z坐标（米）

    // 自定义成员变量和函数
public:
    // 存储所有卫星的星历数据
    CArray<SBroadcastEphemeris, SBroadcastEphemeris> m_arrEphemeris;

    SSatCoordResult m_coordResult;    // 单颗卫星坐标计算结果
    BOOL m_bEphemerisLoaded;          // 星历是否加载成功（标记位）
    // 存储所有计算成功的卫星坐标（用于绘图）
    CArray<SSatCoordResult, SSatCoordResult> m_arrAllSatResults;

    // 字符串分割（按空格分割，处理多空格和制表符）
    void SplitString(const CString& str, CStringArray& arr, const CString& sep);

    // 处理RINEX中的"D格式"科学计数法（如1.234D+05 → 1.234e+05）
    double StrToDoubleWithD(const CString& str);

    // GPS卫星坐标计算（根据广播星历和UTC时间计算卫星地心地固坐标）
    // 参数：
    //   eph    - 广播星历参数
    //   strUTC - UTC时间字符串
    //   result - 输出的卫星坐标结果
    // 返回：计算是否成功（BOOL）
    BOOL CalcGPSSatCoord(const SBroadcastEphemeris& eph, const CString& strUTC, SSatCoordResult& result);

    // UTC时间转GPST时间（返回相对于GPS时0点的秒数）
    double UTC2GPST(const CString& strUTC);

    // 字符串转时间戳（UTC格式）
    time_t StringToTime(const CString& strUTC);

    // 按钮点击事件处理函数
    afx_msg void OnBnClickedBtnOpen();    // 打开广播星历文件
    afx_msg void OnBnClickedBtnCalc();    // 计算所有卫星坐标
    afx_msg void OnBnClickedBtnSave();    // 保存计算结果
    afx_msg void OnBnClickedBtnDraw();    // 绘制卫星轨道图
    afx_msg void OnLvnItemchangedListResult(NMHDR* pNMHDR, LRESULT* pResult); // 列表项变化处理
    afx_msg void OnEnChangeStationCoords(); // 测站坐标编辑框内容变化处理
    afx_msg void OnStnClickedStaticStationTitle(); // 测站标题静态控件点击处理
    afx_msg void OnBnClickedBtnClear();   // 清除数据按钮点击处理
    afx_msg void OnStnClickedStaticCoordsys(); // 坐标系图片控件点击处理
    afx_msg void OnStnClickedStaticStationZ(); // 测站Z坐标静态控件点击处理
    afx_msg void OnBnClickedBtnCalcSingle();  // 单个卫星计算按钮点击处理
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point); // 双击放大卫星图处理
};