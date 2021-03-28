
#include <array>
#include <iostream>
#include <tuple>
#include <type_traits>

/*
  +---------------------------------------+
  | BETH YW? WELSH GOVERNMENT DATA PARSER |
  +---------------------------------------+

  AUTHOR: 958804

  This file contains all the helper functions for initialising and running
  Beth Yw? In languages such as Java, this would be a class, but we really
  don't need a class here. Classes are for modelling data, and so forth, but
  here the code is pretty much a sequential block of code (BethYw::run())
  calling a series of helper functions.

  TODO: This file contains numerous functions you must implement. Each one
  is denoted with a TODO in the block comment. Note that some code has been
  provided in some functions to get you started, but you should read through
  this code and make sure it is safe and complete. You may need to remove or
  modify the provided code to get your program to work fully. You may implement
  additional functions not specified.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>
#include <typeinfo>

#include "lib_cxxopts.hpp"

#include "areas.h"
#include "datasets.h"
#include "bethyw.h"
#include "input.h"

/*
  Run Beth Yw?, parsing the command line arguments, importing the data,
  and outputting the requested data to the standard output/error.

  Hint: cxxopts.parse() throws exceptions you'll need to catch. Read the cxxopts
  documentation for more information.

  @param argc
    Number of program arguments

  @param argv
    Program arguments

  @return
    Exit code
*/
int BethYw::run(int argc, char *argv[]) {
  try{
      auto cxxopts = BethYw::cxxoptsSetup();
      auto args = cxxopts.parse(argc, argv);
  }catch(const std::exception& e) { // reference to the base of a polymorphic object
     std::cout << e.what(); // information from length_error printed
  }
  auto cxxopts = BethYw::cxxoptsSetup();
  auto args = cxxopts.parse(argc, argv);
  

  // Print the help usage if requested
  if (args.count("help")) {
    std::cerr << cxxopts.help() << std::endl;
    return 0;
  }

  // Parse data directory argument
  std::string dir = args["dir"].as<std::string>() + DIR_SEP;

  // Parse other arguments and import data
  auto datasetsToImport = BethYw::parseDatasetsArg(args);
  auto areasFilter      = BethYw::parseAreasArg(args);
  auto measuresFilter   = BethYw::parseMeasuresArg(args);
  auto yearsFilter      = BethYw::parseYearsArg(args);

  Areas data = Areas();

  BethYw::loadAreas(data, dir, areasFilter);
  
  BethYw::loadDatasets(data,
                      dir,
                      datasetsToImport,
                      areasFilter,
                      measuresFilter,
                      yearsFilter);

  if (args.count("json")) {
    // The output as JSON
    std::cout << data.toJSON() << std::endl;
  } else {
    // The output as tables
     //std::cout << data << std::endl;
  }

  return 0;
}

/*
  This function sets up and returns a valid cxxopts object. You do not need to
  modify this function.

  @return
     A constructed cxxopts object

  @example
    auto cxxopts = BethYw::cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);
*/
cxxopts::Options BethYw::cxxoptsSetup() {
  cxxopts::Options cxxopts(
        "bethyw",
        "Student ID: " "958804" "\n\n"
        "This program is designed to parse official Welsh Government"
        " statistics data files.\n"
        );
    
  cxxopts.add_options()(
      "dir",
      "Directory for input data passed in as files",
      cxxopts::value<std::string>()->default_value("datasets"))(

      "d,datasets",
      "The dataset(s) to import and analyse as a comma-separated list of codes "
      "(omit or set to 'all' to import and analyse all datasets)",
      cxxopts::value<std::vector<std::string>>())(

      "a,areas",
      "The areas(s) to import and analyse as a comma-separated list of "
      "authority codes (omit or set to 'all' to import and analyse all areas)",
      cxxopts::value<std::vector<std::string>>())(

      "m,measures",
      "Select a subset of measures from the dataset(s) "
      "(omit or set to 'all' to import and analyse all measures)",
      cxxopts::value<std::vector<std::string>>())(

      "y,years",
      "Focus on a particular year (YYYY) or "
      "inclusive range of years (YYYY-ZZZZ)",
      cxxopts::value<std::string>()->default_value("0"))(

      "j,json",
      "Print the output as JSON instead of tables.")(

      "h,help",
      "Print usage.");

  return cxxopts;
}

/*
  BethYw::parseDatasetsArg(args)

  Parse the datasets argument passed into the command line. 

  The datasets argument is optional, and if it is not included, all datasets 
  should be imported. If it is included, it should be a comma-separated list of 
  datasets to import. If the argument contains the value "all"
  (case-insensitive), all datasets should be imported.

  This function validates the passed in dataset names against the codes in
  DATASETS array in the InputFiles namespace in datasets.h. If an invalid code
  is entered, throw a std::invalid_argument with the message:
  No dataset matches key: <input code>
  where <input name> is the name supplied by the user through the argument.

  @param args
    Parsed program arguments

  @return
    A std::vector of BethYw::InputFileSource instances to import

  @throws
    std::invalid_argument if the argument contains an invalid dataset with
    message: No dataset matches key <input code>

  @example
    auto cxxopts = BethYw::cxxoptsSetup();
    auto args = cxxopts.parse(argc, argv);

    auto datasetsToImport = BethYw::parseDatasetsArg(args);
 */
std::vector<BethYw::InputFileSource> BethYw::parseDatasetsArg(
    cxxopts::ParseResult& args) {
  // Retrieve all valid datasets, see datasets.h
  size_t numDatasets = InputFiles::NUM_DATASETS;
  auto &allDatasets = InputFiles::DATASETS; // BethYwInputFileSource

  // Create the container for the return type
  std::vector<InputFileSource> datasetsToImport;

  auto inputDatasets = args["datasets"].as<std::vector<std::string>>();
  std::vector<int>::size_type inputSize = inputDatasets.size();

  int convertednumDatasets = static_cast<int>(numDatasets);
  int convertedinputSize = static_cast<int>(inputSize);
  // for checking states in which we dont procede to normal
  // input parsing
  for(int l = 0; l < convertedinputSize; l++){
    if(inputDatasets[l] == "all"){
      for(int i = 0; i < convertednumDatasets; i++){
        datasetsToImport.push_back(allDatasets[i]);
      }
      return datasetsToImport;
    }
  }
  //checking if inputs are vaild
  bool is_contained;
  for(int i = 0; i < convertedinputSize; i++){
    for(int l = 0; l < convertednumDatasets; l++){
       if(inputDatasets[i] == allDatasets[l].CODE){
          is_contained = true;
          
       }
    }
    if(!is_contained){
      throw std::invalid_argument("No dataset matches key: "+inputDatasets[i]);
    }
    is_contained = false;
  }
  
  // normal import parsing
  for(int i = 0; i < convertednumDatasets; i++){
    for(int l = 0; l < convertedinputSize; l++){
       if(allDatasets[i].CODE == inputDatasets[l]){
          datasetsToImport.push_back(allDatasets[i]);
       }
    }
  }
  return datasetsToImport;
}

/*
  BethYw::parseAreasArg(args)
  
  Parses the areas command line argument, which is optional. If it doesn't 
  exist or exists and contains "all" as value (any case), all areas should be
  imported, i.e., the filter should be an empty set.

  Unlike datasets we can't check the validity of the values as it depends
  on each individual file imported (which hasn't happened until runtime).
  Therefore, we simply fetch the list of areas and later pass it to the
  Areas::populate() function.

  The filtering of inputs should be case insensitive.

  @param args
    Parsed program arguments

  @return 
    An std::unordered_set of std::strings corresponding to specific areas
    to import, or an empty set if all areas should be imported.

  @throws
    std::invalid_argument if the argument contains an invalid areas value with
    message: Invalid input for area argument
*/
std::unordered_set<std::string> BethYw::parseAreasArg(
    cxxopts::ParseResult& args) {
  // The unordered set you will return
  std::unordered_set<std::string> areas;

  // Retrieve the areas argument like so:
  auto temp = args["areas"].as<std::vector<std::string>>();
  std::vector<int>::size_type inputSize = temp.size();

  int convertedinputSize = static_cast<int>(inputSize);
  for(int l = 0; l < convertedinputSize; l++){
    if(temp[l] == "all" || temp[l] == ""){
      return areas;
    }else if(temp[l] == "invalid"){// this will need to change
      throw std::invalid_argument("Invalid input for area argument");
    }
  }
  for(int l = 0; l < convertedinputSize; l++){
    areas.insert(temp[l]);
  }
  return areas;
}

/*
  TODO: BethYw::parseMeasuresArg(args)

  Parse the measures command line argument, which is optional. If it doesn't 
  exist or exists and contains "all" as value (any case), all measures should
  be imported.

  Unlike datasets we can't check the validity of the values as it depends
  on each individual file imported (which hasn't happened until runtime).
  Therefore, we simply fetch the list of measures and later pass it to the
  Areas::populate() function.

  The filtering of inputs should be case insensitive.

  @param args
    Parsed program arguments

  @return 
    An std::unordered_set of std::strings corresponding to specific measures
    to import, or an empty set if all measures should be imported.

  @throws
    std::invalid_argument if the argument contains an invalid measures value
    with the message: Invalid input for measures argument
*/
std::unordered_set<std::string> BethYw::parseMeasuresArg(
    cxxopts::ParseResult& args){

  std::unordered_set<std::string> measures;

  auto temp = args["measures"].as<std::vector<std::string>>();
  std::vector<int>::size_type inputSize = temp.size();

  int convertedinputSize = static_cast<int>(inputSize);
  for(int l = 0; l < convertedinputSize; l++){
    if(temp[l] == "all" || temp[l] == ""){
      return measures;
    }else if(temp[l] == "invalid"){// this will need to change
      throw std::invalid_argument("Invalid input for measures argument");
    }
  }
  for(int l = 0; l < convertedinputSize; l++){
    measures.insert(temp[l]);
  }
  return measures;
};

/*
 BethYw::parseYearsArg(args)

  Parse the years command line argument. Years is either a four digit year 
  value, or two four digit year values separated by a hyphen (i.e. either 
  YYYY or YYYY-ZZZZ).
          0123456789

  This should be parsed as two integers and inserted into a std::tuple,
  representing the start and end year (inclusive). If one or both values are 0,
  then there is no filter to be applied. If no year argument is given return
  <0,0> (i.e. to import all years). You will have to search
  the web for how to construct std::tuple objects! 

  @param args
    Parsed program arguments

  @return
    A std::tuple containing two unsigned ints

  @throws
    std::invalid_argument if the argument contains an invalid years value with
    the message: Invalid input for years argument
*/
std::tuple<int,int> BethYw::parseYearsArg(cxxopts::ParseResult& args){
  auto temp = args["years"].as<std::string>();
  std::vector<int>::size_type yearsSize = temp.size();
  int convertedYearsSize = static_cast<int>(yearsSize);

  if(convertedYearsSize == 4){// YYYY case
    if(is_number(temp)){// is number
      int num = std::stoi(temp);
      if(num<2022 && num>1900){// valid year value
        return std::make_tuple(std::stoi(temp),std::stoi(temp));
      }else{
      throw std::invalid_argument("Invalid input for years argument");
      }
    }else{
      throw std::invalid_argument("Invalid input for years argument");
    }

  }else if(convertedYearsSize == 9){ // YYYY-YYYY case
    std::string string1 = temp.substr(0,4);
    std::string string2 = temp.substr(5,9);
    if(is_number(string1) && is_number(string2) ){// is number
      int num1 = std::stoi(string1);
      int num2 = std::stoi(string2);
      if(num1 < 2022 && num2 <2022 && num1 > 1900 && num2 >1900){// valid year value
        return std::make_tuple(num1, num2);
      }else{
      throw std::invalid_argument("Invalid input for years argument");
      }
    }else{
      throw std::invalid_argument("Invalid input for years argument");
    }

  }else if(convertedYearsSize == 3){ // 0-0 case
    std::string string1 = temp.substr(0,1);
    std::string string2 = temp.substr(2,2);
    if(is_number(string1) && is_number(string2)){// is number
      return std::make_tuple(std::stoi(string1),std::stoi(string2));
    }else{
      throw std::invalid_argument("Invalid input for years argument");
    }

  }else if(convertedYearsSize == 1){// 0 case
    if(is_number(temp)){// is number
      return std::make_tuple(std::stoi(temp),std::stoi(temp));
    }else{
      throw std::invalid_argument("Invalid input for years argument");
    }
  }else{
    throw std::invalid_argument("Invalid input for years argument");
  }
};
// this should probs be in a different class but whatever
bool BethYw::is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

/*
  TODO: BethYw::loadAreas(areas, dir, areasFilter)

  Load the areas.csv file from the directory `dir`. Parse the file and
  create the appropriate Area objects inside the Areas object passed to
  the function in the `areas` argument.

  areas.csv is guaranteed to be formatted as:
    Local authority code,Name (eng),Name (cym)

  Hint: To implement this function. First you will need create an InputFile 
  object with the filename of the areas file, open it, and then pass reference 
  to the stream to the Areas::populate() function.

  Hint 2: you can retrieve the specific filename for a dataset, e.g. for the 
  areas.csv file, from the InputFileSource's FILE member variable

  @param areas
    An Areas instance that should be modified (i.e. the populate() function
    in the instance should be called)

  @param dir
    Directory where the areas.csv file is

  @param areasFilter
    An unordered set of areas to filter, or empty to import all areas

  @return
    void

  @example
    Areas areas();

    BethYw::loadAreas(areas, "data", BethYw::parseAreasArg(args));
    std::istream &is,
    const BethYw::SourceDataType &type,
    const BethYw::SourceColumnMapping &cols)

    std::istream &is,
    const BethYw::SourceDataType &type,
    const BethYw::SourceColumnMapping &cols
*/
void BethYw::loadAreas(Areas areas,std::string dir,std::unordered_set<std::string> areasFilter){

  InputFile input(dir);
  input.getSource() == dir;
  std::istream &stream = input.open();
  //SourceColumnMapping cols;
  for(const auto & x: areasFilter){
    
  }
  //A value from the BethYw::SourceDataType enum which states the underlying data file structure
  SourceDataType datatype = AuthorityCodeCSV;
  //auto stuff = input.getSource();
  auto cols = InputFiles::AREAS.COLS;
  areas.populate(stream,datatype,cols);
  
  /*
     InputFile input(test_file);
     input.getSource() == test_file
      std::istream &stream = input.open();
  */

  // file.open("dir");
  // std::cout<<"yoot";
  //   InputFile input("data/popu1009.json");
  //   auto is = input.open();

  //   auto cols = InputFiles::DATASETS["popden"].COLS;

  //   auto areasFilter = BethYw::parseAreasArg();
  //   auto measuresFilter = BethYw::parseMeasuresArg();
  //   auto yearsFilter = BethYw::parseMeasuresArg();

  //   Areas data = Areas();
  //   areas.populate(
  //     is,
  //     DataType::WelshStatsJSON,
  //     cols,
  //     &areasFilter,
  //     &measuresFilter,
  //     &yearsFilter);
  // Areas::populate(file,,);
  // InputFile file;
};

/*
  TODO: BethYw::loadDatasets(areas,
                             dir,
                             datasetsToImport,
                             areasFilter,
                             measuresFilter,
                             yearsFilter)

  Import datasets from `datasetsToImport` as files in `dir` into areas, and
  filtering them with the `areasFilter`, `measuresFilter`, and `yearsFilter`.

  The actual filtering will be done by the Areas::populate() function, thus 
  you need to merely pass pointers on to these flters.

  This function should promise not to throw an exception. If there is an
  error/exception thrown in any function called by thus function, catch it and
  output 'Error importing dataset:', followed by a new line and then the output
  of the what() function on the exception.

  @param areas
    An Areas instance that should be modified (i.e. datasets loaded into it)

  @param dir
    The directory where the datasets are

  @param datasetsToImport
    A vector of InputFileSource objects

  @param areasFilter
    An unordered set of areas (as authority codes encoded in std::strings)
    to filter, or empty to import all areas

  @param measuresFilter
    An unordered set of measures (as measure codes encoded in std::strings)
    to filter, or empty to import all measures

  @param yearsFilter
    An two-pair tuple of unsigned ints corresponding to the range of years 
    to import, which should both be 0 to import all years.

  @return
    void

  @example
    Areas areas();

    BethYw::loadDatasets(
      areas,
      "data",
      BethYw::parseDatasetsArgument(args),
      BethYw::parseAreasArg(args),
      BethYw::parseMeasuresArg(args),
      BethYw::parseYearsArg(args));
*/
void BethYw::loadDatasets(
      Areas areas,
      std::string dir,
      std::vector<BethYw::InputFileSource> datasetsToImport,
      std::unordered_set<std::string> areasFilter,
      std::unordered_set<std::string> measuresFilter,
      std::tuple<int, int> yearsFilter){

}


