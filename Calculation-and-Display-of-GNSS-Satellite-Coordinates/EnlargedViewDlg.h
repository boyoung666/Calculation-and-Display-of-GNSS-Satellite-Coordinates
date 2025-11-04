#pragma once
// 放大视图对话框类头文件

// MFC基础窗口
#include "afxwin.h"
#include "Gdiplus.h"
#include "resource.h"
#include "SatelliteCoordCalcDlg.h"

// 使用GDI+命名空间
using namespace Gdiplus;

// 放大视图对话框类：显示卫星坐标的放大图形界面
class CEnlargedViewDlg : public CDialogEx
{
public:
    // 构造函数
    // 参数：pParent 父窗口指针，默认为nullptr
    CEnlargedViewDlg(CWnd* pParent = nullptr);

    // 析构函数
    virtual ~CEnlargedViewDlg();

    // 对话框资源ID
    enum { IDD = IDD_ENLARGED_VIEW };

    // 设置绘图数据
    // 参数：
    //   pImage     坐标系背景图片指针
    //   arrResults 卫星坐标结果数组（包含PRN和XYZ坐标）
    void SetData(Image* pImage, const CArray<SSatCoordResult, SSatCoordResult>& arrResults);

protected:
    // 数据交换函数：对话框与变量间的数据传递
    // 参数：pDX 数据交换对象指针
    virtual void DoDataExchange(CDataExchange* pDX);

    // 消息映射声明
    DECLARE_MESSAGE_MAP()

private:
    // 成员变量
    Image* m_pCoordsysImage;                  // 坐标系背景图片指针
    CArray<SSatCoordResult, SSatCoordResult> m_arrSatResults;  // 卫星坐标结果数组
    std::map<char, Color> m_satTypeColors;    // 卫星类型与颜色的映射表

    // 私有函数
    // 初始化卫星类型颜色映射（为不同系统卫星分配颜色）
    void InitSatelliteColors();

    // 绘制图例（显示卫星类型与颜色的对应关系）
    // 参数：
    //   pGraphics GDI+绘图对象指针
    //   rect      对话框客户区矩形（用于定位图例）
    void DrawLegend(Graphics* pGraphics, CRect& rect);

    // 绘图事件处理函数：绘制背景、卫星点和图例
    afx_msg void OnPaint();
};