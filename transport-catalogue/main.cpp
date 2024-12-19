#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <fstream> 
#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>

using namespace std;

#include "request_handler.h"
#include "svg.h"
#include "map_renderer.h"

using namespace std::literals;
using namespace svg;

void ExampleIn() {

    std::ifstream in("example_in.txt");
    assert(in.is_open());

    std::ofstream out;
    out.open("example_out.txt");
    assert(out.is_open());

    std::ofstream out_svg;
    out_svg.open("example_out.svg");
    assert(out.is_open());

    transportcatalogue::TransportCatalogue catalogue;
    RequestHandler request_handler(catalogue);
    request_handler.Load(in);
    request_handler.UploadAnswers(out);
    request_handler.RenderMap(out_svg);
}

int main() {
    ExampleIn();
}