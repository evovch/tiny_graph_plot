//#include <cstdio>

#include "tiny_graph_plot.h"

float f1(const float x) { return (x * x); }
float f2(const float x) { return (2.0f - x); }
float f3(const float x) { return (0.5f * x * x * x - x * x + x + 1.0f); }
float f4(const float x) { return (1.0f / (x + 4.0f)); }
float f5(const float x) { return 2.0f * sinf(x / 1.0f); }
float f6(const float x) { return 2.0f * cosf(x / 1.0f); }

int main(int argc, char** argv)
{
    // ===========================================================================
    // Use either this to work with single precision
    tiny_graph_plot::GraphManager<float>& graph_manager = global_graph_manager_float;
    tiny_graph_plot::CanvasManager<float>& canvas_manager = global_canvas_manager_float;
    typedef tiny_graph_plot::Graph<float> Graph;
    typedef tiny_graph_plot::Histogram1d<float, unsigned long> Histogram1d;
    typedef tiny_graph_plot::Canvas<float> Canvas;
    typedef tiny_graph_plot::Vec2<float> Vec2;
    // or this to work with double precision
    //tiny_graph_plot::GraphManager<double>& graph_manager = global_graph_manager_double;
    //tiny_graph_plot::CanvasManager<double>& canvas_manager = global_canvas_manager_double;
    //typedef tiny_graph_plot::Graph<double> Graph;
    //typedef tiny_graph_plot::Histogram1d<double, unsigned long> Histogram1d;
    //typedef tiny_graph_plot::Canvas<double> Canvas;
    //typedef tiny_graph_plot::Vec2<double> Vec2;
    // ===========================================================================

    // Test set 1
    const float xmin = -5.0f;
    const float xmax = 5.0f;
    const int N = 20001;
    const float dx = (xmax - xmin) / (float)(N - 1);
    Vec2* xy1 = new Vec2[N];
    Vec2* xy2 = new Vec2[N];
    Vec2* xy3 = new Vec2[N];
    Vec2* xy4 = new Vec2[N];

    for (int i = 0; i < N; i++) {
        const float x = xmin + dx * (float)i;
        const float y1 = f1(x);
        const float y2 = f2(x);
        const float y3 = f3(x);
        const float y4 = f4(x);
        xy1[i][0] = x;
        xy1[i][1] = y1;
        xy2[i][0] = x;
        xy2[i][1] = y2;
        xy3[i][0] = x;
        xy3[i][1] = y3;
        xy4[i][0] = x;
        xy4[i][1] = y4;

        //xy1[i][0] = (float)rand() / (float)RAND_MAX;
        //xy1[i][1] = (float)rand() / (float)RAND_MAX;

        //printf("% 0.6f\t% 0.6f\t% 0.6f\t% 0.6f\t% 0.6f\n", x, y1, y2, y3, y4);
    }

    // Test set 2
    const float xmin2 = -2.0f;
    const float xmax2 = 5.0f;
    const int N2 = 201;
    const float dx2 = (xmax2 - xmin2) / (float)(N2 - 1);
    Vec2* xy5 = new Vec2[N2];
    Vec2* xy6 = new Vec2[N2];

    for (int i = 0; i < N2; i++) {
        const float x = xmin2 + dx2 * (float)i;
        const float y5 = f5(x);
        const float y6 = f6(x);
        xy5[i][0] = x;
        xy5[i][1] = y5;
        xy6[i][0] = x;
        xy6[i][1] = y6;
    }

    // Test set 3
    const float xmin3 = 0.0f;
    const float xmax3 = 10.0f;
    const int N3 = 1001;
    const float dx3 = (xmax3 - xmin3) / (float)(N3 - 1);
    Vec2* xy7 = new Vec2[N3];
    const float a = 100.0f;
    const float b = 5.0f;
    const float c = 2.0f;
    const float k = -0.5f / (c * c);
    for (int i = 0; i < N3; i++) {
        const float x = xmin3 + dx3 * (float)i;
        const float y7 = a * exp(k * (x - b) * (x - b));
        xy7[i][0] = x;
        xy7[i][1] = y7;
    }

    Graph& gr1 = graph_manager.CreateGraph();
    gr1.SetSharedBuffer(N, xy1);
    gr1.SetColor(tiny_gl_text_renderer::colors::red);
    gr1.SetLineWidth(6.0f);

    Graph& gr2 = graph_manager.CreateGraph();
    gr2.SetSharedBuffer(N, xy2);
    gr2.SetColor(tiny_gl_text_renderer::colors::yellow);

    Graph& gr3 = graph_manager.CreateGraph();
    gr3.SetSharedBuffer(N, xy3);
    gr3.SetColor(tiny_gl_text_renderer::colors::magenta);

    Graph& gr4 = graph_manager.CreateGraph();
    gr4.SetSharedBuffer(N, xy4);
    gr4.SetColor(tiny_gl_text_renderer::colors::cyan);

    Graph& gr5 = graph_manager.CreateGraph();
    gr5.SetSharedBuffer(N2, xy5);
    gr5.SetColor(tiny_gl_text_renderer::colors::green);

    Graph& gr6 = graph_manager.CreateGraph();
    gr6.SetSharedBuffer(N2, xy6);
    gr6.SetColor(tiny_gl_text_renderer::colors::blue);

    Graph& gr7 = graph_manager.CreateGraph();
    gr7.SetSharedBuffer(N3, xy7);
    gr7.SetColor(tiny_gl_text_renderer::colors::lime);

    Histogram1d& histo1 = graph_manager.CreateHistogram1d();
    // nbins, xmin, xmax, a, b, c
    histo1.GenGauss(20, 0.0f, 10.0f, 100.0f, 5.0f, 2.0f);

    Canvas& canv1 = canvas_manager.CreateCanvas("canv1", 800, 800, 100, 100);

    // Optional settings. You can completely omit this section and use default values.
    canv1.SetXaxisTitle("x axis");
    canv1.SetYaxisTitle("y axis", true); // Second argument - rotated by 90deg
    canv1.EnableHgrid();   // canv1.DisableHgrid();
    canv1.EnableVgrid();   // canv1.DisableVgrid();
    canv1.EnableAxes();    // canv1.DisableAxes();
    canv1.EnableVref();    // canv1.DisableVref();
    canv1.EnableFrame();   // canv1.DisableFrame();
    canv1.EnableCursor();  // canv1.DisableCursor();
    canv1.EnableCircles(); // canv1.DisableCircles();

    // Choose the color scheme
    canv1.SetDarkColorScheme(); // canv1.SetBrightColorScheme();
    // Or set individual colors (example values here - dark color scheme)
    canv1.SetBackgroundColor       (tiny_gl_text_renderer::colors::gray1);
    canv1.SetInFrameBackgroundColor(tiny_gl_text_renderer::colors::gray05);
    canv1.SetHGridFineColor  (tiny_gl_text_renderer::colors::gray6);
    canv1.SetVGridFineColor  (tiny_gl_text_renderer::colors::gray6);
    canv1.SetHGridCoarseColor(tiny_gl_text_renderer::colors::gray5);
    canv1.SetVGridCoarseColor(tiny_gl_text_renderer::colors::gray5);
    canv1.SetAxesColor  (tiny_gl_text_renderer::colors::gray75);
    canv1.SetVrefColor  (tiny_gl_text_renderer::colors::olive);
    canv1.SetFrameColor (tiny_gl_text_renderer::colors::gray5);
    canv1.SetCursorColor(tiny_gl_text_renderer::colors::white);
    canv1.SetTextColor  (tiny_gl_text_renderer::colors::white);

    canv1.SetHGridFineLineWidth(1.0f);
    canv1.SetVGridFineLineWidth(1.0f);
    canv1.SetHGridCoarseLineWidth(1.0f);
    canv1.SetVGridCoarseLineWidth(1.0f);
    canv1.SetAxesLineWidth(3.0f);
    canv1.SetVrefLineWidth(3.0f);
    canv1.SetFrameLineWidth(3.0f);
    canv1.SetCursorLineWidth(1.0f);
    canv1.SetCircleRadius(10);
    // Use default font size and corresponding window margins
    //canv1.SetFontSize(1.0f);
    //canv1.SetAllMargins(280, 22, 34, 22);
    // Or use twice as small font and corresponding window margins
    const float font_k = 0.51f;
    canv1.SetFontSize(font_k);
    canv1.SetAllMargins((unsigned int)(font_k * 280), (unsigned int)(font_k * 22),
        (unsigned int)(font_k * 34), (unsigned int)(font_k * 22));
    // End of optional settings

    canv1.AddGraph(gr1);
    canv1.AddGraph(gr2);
    canv1.AddGraph(gr3);
    canv1.AddGraph(gr4);
    canv1.AddGraph(gr5);
    canv1.AddGraph(gr6);
    canv1.AddGraph(gr7);
    canv1.AddHistogram(histo1);

    //Canvas& canv2 = canvas_manager.CreateCanvas("canv2", 800, 800, 900, 100);
    //canv2.SetBrightColorScheme();

    //canv2.AddGraph(gr5);
    //canv2.AddGraph(gr6);

    canv1.Show();
    //canv2.Show();

    canvas_manager.WaitForTheWindowsToClose(); // Endless loop

    delete[] xy1;
    delete[] xy2;
    delete[] xy3;
    delete[] xy4;
    delete[] xy5;
    delete[] xy6;
    delete[] xy7;

    return 0;
}
