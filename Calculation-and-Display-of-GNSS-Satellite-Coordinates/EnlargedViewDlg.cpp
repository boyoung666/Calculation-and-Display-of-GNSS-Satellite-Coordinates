// 放大视图对话框实现文件
#include "pch.h"
#include "resource.h"
#include "EnlargedViewDlg.h"

// 构造函数：初始化对话框并设置卫星类型颜色映射
CEnlargedViewDlg::CEnlargedViewDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_ENLARGED_VIEW, pParent)
    , m_pCoordsysImage(nullptr)  // 初始化坐标系图片指针为空
{
    // 初始化卫星类型与颜色的对应关系
    InitSatelliteColors();
}

// 析构函数：默认析构（图片由外部管理，无需额外资源释放）
CEnlargedViewDlg::~CEnlargedViewDlg()
{
}

// 数据交换函数：处理对话框与变量间的数据传递
void CEnlargedViewDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

// 消息映射：关联控件事件与处理函数
BEGIN_MESSAGE_MAP(CEnlargedViewDlg, CDialogEx)
    ON_WM_PAINT()  // 关联绘图事件
END_MESSAGE_MAP()

// 设置绘图数据：接收坐标系图片和卫星坐标结果
// 参数：
//   pImage      坐标系背景图片
//   arrResults  卫星坐标结果数组（包含PRN和XYZ坐标）
void CEnlargedViewDlg::SetData(Image* pImage, const CArray<SSatCoordResult, SSatCoordResult>& arrResults)
{
    m_pCoordsysImage = pImage;               // 保存背景图片指针
    m_arrSatResults.Copy(arrResults);        // 复制卫星坐标结果
}

// 初始化卫星类型颜色映射：为不同系统卫星分配独特颜色
void CEnlargedViewDlg::InitSatelliteColors()
{
    m_satTypeColors['G'] = Color(255, 255, 0, 0);      // GPS (G) - 红色
    m_satTypeColors['C'] = Color(255, 0, 255, 0);      // 北斗 (C) - 绿色
    m_satTypeColors['R'] = Color(255, 0, 0, 255);      // GLONASS (R) - 蓝色
    m_satTypeColors['E'] = Color(255, 255, 255, 0);    // Galileo (E) - 黄色
    m_satTypeColors['S'] = Color(255, 255, 0, 255);    // 其他系统 (S) - 紫色
    m_satTypeColors['I'] = Color(255, 128, 0, 128);    // 印度IRNSS (I) - 紫红色
}

// 绘制图例：在右上角显示卫星类型与颜色的对应说明
// 参数：
//   pGraphics  GDI+绘图对象
//   rect       对话框客户区矩形（用于定位图例）
void CEnlargedViewDlg::DrawLegend(Graphics* pGraphics, CRect& rect)
{
    // 合法性检查：颜色映射为空或绘图对象无效时直接返回
    if (m_satTypeColors.empty() || !pGraphics)
        return;

    // 图例布局参数
    int legendX = rect.right - 150;          // 图例左上角X坐标（右对齐）
    int legendY = 20;                        // 图例左上角Y坐标（顶部偏移）
    int itemHeight = 20;                     // 每个图例项的高度
    int boxSize = 12;                        // 颜色方块的尺寸
    int legendWidth = 150;                   // 图例总宽度
    int legendHeight = m_satTypeColors.size() * itemHeight + 30;  // 图例总高度

    // 绘制图例背景（浅灰色半透明）和边框（灰色）
    SolidBrush bgBrush(Color(240, 240, 240, 240));  // 背景色（R,G,B,透明度）
    Pen borderPen(Color(180, 180, 180), 1);         // 边框（颜色，线宽）
    pGraphics->FillRectangle(&bgBrush,
        (REAL)(legendX - 10), (REAL)(legendY - 10),  // 左上角坐标（修正偏移）
        (REAL)legendWidth, (REAL)legendHeight);      // 宽高
    pGraphics->DrawRectangle(&borderPen,
        (REAL)(legendX - 10), (REAL)(legendY - 10),
        (REAL)legendWidth, (REAL)legendHeight);

    // 绘制图例标题“卫星类型”
    Gdiplus::Font titleFont(L"Arial", 10, FontStyleBold);  // 标题字体（Arial,10号,粗体）
    SolidBrush textBrush(Color::Black);                     // 文字颜色（黑色）
    pGraphics->DrawString(L"卫星类型", -1, &titleFont,
        PointF((REAL)legendX, (REAL)(legendY - 8)), &textBrush);  // 标题位置
    legendY += 15;  // 标题下方偏移，预留空间

    // 绘制每个卫星类型的图例项（颜色方块+文字说明）
    Gdiplus::Font font(L"Arial", 9);  // 图例文字字体类型、字号
    int index = 0;                    // 图例项索引

    // 遍历所有卫星类型，绘制对应图例
    for (auto& pair : m_satTypeColors)
    {
        // 绘制颜色方块
        SolidBrush colorBrush(pair.second);  // 使用当前卫星类型的颜色
        pGraphics->FillRectangle(&colorBrush,
            (REAL)legendX, (REAL)(legendY + index * itemHeight),  // 方块位置
            (REAL)boxSize, (REAL)boxSize);                        // 方块尺寸
        pGraphics->DrawRectangle(&borderPen,  // 绘制方块边框
            (REAL)legendX, (REAL)(legendY + index * itemHeight),
            (REAL)boxSize, (REAL)boxSize);

        // 设置图例文字说明
        CString strDesc;
        switch (pair.first)  // 根据卫星类型首字母设置说明文字
        {
        case 'G': strDesc = L"GPS (G)";     break;
        case 'C': strDesc = L"北斗 (C)";    break;
        case 'R': strDesc = L"GLONASS (R)"; break;
        case 'E': strDesc = L"Galileo (E)"; break;
        case 'S': strDesc = L"其他 (S)";    break;
        case 'I': strDesc = L"IRNSS (I)";   break;
        default:  strDesc.Format(_T("%c 系统"), pair.first);  // 未知类型默认格式
        }

        // 绘制图例文字（位于颜色方块右侧）
        pGraphics->DrawString(strDesc.GetString(), -1, &font,
            PointF((REAL)(legendX + boxSize + 5), (REAL)(legendY + index * itemHeight)),
            &textBrush);

        index++;  // 移动到下一个图例项
    }
}

// 绘图事件处理：绘制背景图片、卫星点和图例
void CEnlargedViewDlg::OnPaint()
{
    CPaintDC dc(this);               // 获取设备上下文
    Graphics graphics(dc.GetSafeHdc());  // 创建GDI+绘图对象

    CRect rect;
    GetClientRect(&rect);  // 获取对话框客户区大小

    // 绘制坐标系背景图片（铺满客户区）
    if (m_pCoordsysImage && m_pCoordsysImage->GetLastStatus() == Ok)
    {
        graphics.DrawImage(m_pCoordsysImage,
            0, 0, rect.Width(), rect.Height());  // 拉伸图片适配客户区
    }

    // 绘制卫星点（若有有效数据）
    if (!m_arrSatResults.IsEmpty())
    {
        // 步骤1：计算卫星坐标的最大/最小值（用于坐标映射到屏幕）
        double minX = 1e20, maxX = -1e20;  // X坐标范围初始化（极大/极小值）
        double minZ = 1e20, maxZ = -1e20;  // Z坐标范围初始化
        for (int i = 0; i < m_arrSatResults.GetSize(); i++)
        {
            SSatCoordResult sat = m_arrSatResults[i];
            minX = min(minX, sat.X);  // 更新X最小值
            maxX = max(maxX, sat.X);  // 更新X最大值
            minZ = min(minZ, sat.Z);  // 更新Z最小值
            maxZ = max(maxZ, sat.Z);  // 更新Z最大值
        }

        // 步骤2：计算缩放比例和偏移量（将卫星坐标映射到屏幕坐标）
        double imgWidth = rect.Width() * 0.8;   // 有效绘图区域宽度
        double imgHeight = rect.Height() * 0.8; // 有效绘图区域高度
        double scaleX = imgWidth / (maxX - minX);  // X方向缩放比例
        double scaleZ = imgHeight / (maxZ - minZ); // Z方向缩放比例
        double scale = min(scaleX, scaleZ);        // 取较小的缩放比例（避免拉伸）
        double offsetX = rect.Width() * 0.1;       // X方向偏移
        double offsetY = rect.Height() * 0.1;      // Y方向偏移

        // 坐标转换lambda表达式：将卫星坐标(x,z)转换为屏幕像素坐标
        auto toPixelX = [&](double x) { return (x - minX) * scale + offsetX; };
        auto toPixelY = [&](double z) { return (maxZ - z) * scale + offsetY; };  // Z轴反向（屏幕Y向下）

        // 步骤3：绘制卫星点和PRN编号
        Gdiplus::Font fontPRN(L"宋体", 10);          // PRN文字字体类型、字号
        SolidBrush brushPRNText(Color(0, 0, 0));      // PRN文字颜色
        Color defaultColor = Color(255, 128, 128, 128); // 未知类型卫星默认颜色

        // 遍历所有卫星，绘制对应的点和编号
        for (int i = 0; i < m_arrSatResults.GetSize(); i++)
        {
            SSatCoordResult sat = m_arrSatResults[i];
            if (sat.strPRN.IsEmpty())  // 跳过PRN为空的无效数据
                continue;

            // 获取卫星类型对应的颜色
            char type = toupper(sat.strPRN[0]);  // 取PRN首字母（大写）
            Color satColor = defaultColor;
            if (m_satTypeColors.find(type) != m_satTypeColors.end())
            {
                satColor = m_satTypeColors[type];  // 使用预定义颜色
            }

            // 绘制卫星点（圆形）
            SolidBrush brushSat(satColor);  // 卫星点颜色
            float pixelX = (float)toPixelX(sat.X);  // 转换X坐标到屏幕
            float pixelY = (float)toPixelY(sat.Z);  // 转换Z坐标到屏幕
            graphics.FillEllipse(&brushSat,
                pixelX - 8.0f, pixelY - 8.0f,  // 圆形左上角坐标
                16.0f, 16.0f);                 // 圆形大小

            // 绘制卫星PRN编号（位于点的右侧）
            graphics.DrawString(sat.strPRN.GetString(),
                sat.strPRN.GetLength(),
                &fontPRN,
                PointF(pixelX + 10, pixelY - 5),  // 文字位置
                &brushPRNText);
        }

        // 步骤4：绘制图例
        DrawLegend(&graphics, rect);
    }
}