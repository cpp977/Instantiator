#include <fmt/color.h>
#include <fmt/core.h>
#include <string>

struct ProgressStyle
{
    fmt::color fg_done = fmt::color::lime;
    fmt::color bg_done = fmt::color::sea_green;

    fmt::color fg_do = fmt::color::dark_blue;
    fmt::color bg_do = fmt::color::black;

    fmt::color fg_percent = fmt::color::lime;
    fmt::color bg_percent = fmt::color::black;

    fmt::color fg_message = fmt::color::white_smoke;
    fmt::color bg_message = fmt::color::black;
};

inline ProgressStyle DefaultStyle = ProgressStyle();
inline ProgressStyle FancyStyle = ProgressStyle{fmt::color::pink,
                                                fmt::color::sea_green,
                                                fmt::color::alice_blue,
                                                fmt::color::black,
                                                fmt::color::dark_slate_blue,
                                                fmt::color::black,
                                                fmt::color::white_smoke,
                                                fmt::color::yellow_green};

class ProgressBar
{
private:
    int tasks = 100;
    std::string fill_done = "█";
    std::string fill_do = "-";

    ProgressStyle style;

    int curr = 0;
    double scaling;

public:
    ProgressBar(int tasks = 100, const std::string& fill_done = "█", const std::string& fill_do = "-", ProgressStyle style = DefaultStyle)
        : tasks(tasks)
        , fill_done(fill_done)
        , fill_do(fill_do)
        , style(style)
    {
        scaling = 100. / this->tasks;
    }

    int numTasks() const { return tasks; }

    void step(std::string message = "");
    void update(int curr, std::string message = "");

private:
    void draw(std::string message = "") const;
};
