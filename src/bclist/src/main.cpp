#include <filesystem>
#include <fstream>

#include <fmt/core.h>
#include <fmt/os.h>

#include <args.hxx>

#include "bclist.hpp"

namespace fs = std::filesystem;

void print_info(std::string_view str, bclist::options o = bclist::options{}) {
  fs::path filename = str;

  if (!fs::is_regular_file(filename)) {
    fmt::print(stderr, "The path isn't a file.\n");
    return;
  }
  std::ifstream luac(filename, std::ios::binary);
  if (luac.fail()) {
    fmt::print(stderr, "Error opening file.\n");
    return;
  }
  dislua::buffer buf((std::istreambuf_iterator<char>(luac)), std::istreambuf_iterator<char>());
  auto info = dislua::read_all(buf);
  luac.close();
  if (!info) {
    fmt::print(stderr, "Unknown compiler of lua script.\n");
    return;
  }

  fs::path new_filename = filename.stem();
  new_filename += fs::path("-bclist.lua");
  filename.replace_filename(new_filename);

  auto list = bclist::get_list(info.get());
  list->option = o;
  list->update();

  auto out = fmt::output_file(filename.string());
  out.print("{}", list->full());
}

int main(int argc, char *argv[]) {
  args::ArgumentParser parser{"bclist-cli: Print the bytecode list of the compiled Lua script."};
  args::HelpFlag h{parser, "help", "Display the help menu", {'h', "help"}};
  args::ValueFlag<std::string> input{parser, "file", "Input file", {'i', "input"}};

  args::Group bcoptions{parser, "Options for bclist's output:"};
  args::ValueFlag<bool> show_file_offsets{bcoptions, "show", "Show offsets in the script", {"file-offsets"}, false};
  args::ValueFlag<size_t> max_length{bcoptions, "length", "Maximum line length", {"max-length"}, 0};

  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Completion &e) {
    std::cout << e.what();
    return 0;
  } catch (const args::Help &) {
    std::cout << parser;
    return 0;
  } catch (const args::ParseError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  if (input) {
    bclist::options o;
    // o.show_file_offsets = show_file_offsets.Get();
    o.max_length = max_length.Get();

    print_info(input.Get(), o);
  }

  return 0;
}