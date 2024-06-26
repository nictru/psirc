#include <string>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <getopt.h>
#include <thread>
#include <time.h>
#include <array>

#include <cstdio>

#include <zlib.h>

#include "common.h"
#include "ProcessReads.h"
#include "KmerIndex.h"
#include "Kmer.hpp"
#include "MinCollector.h"
#include "EMAlgorithm.h"
#include "weights.h"
#include "Inspect.h"
#include "Bootstrap.h"
#include "H5Writer.h"
#include "GeneModel.h"


#define PSIRC_QUANT_VERSTION "1.0"
//#define ERROR_STR "\033[1mError:\033[0m"
#define ERROR_STR "Error:"

using namespace std;


int my_mkdir(const char *path, mode_t mode) {
  #ifdef _WIN64
  return mkdir(path);
  #else
  return mkdir(path,mode);
  #endif
}


void ParseOptionsIndex(int argc, char **argv, ProgramOptions& opt) {
  int verbose_flag = 0;
  int make_unique_flag = 0;
  const char *opt_string = "i:k:";
  static struct option long_options[] = {
    // long args
    {"verbose", no_argument, &verbose_flag, 1},
    {"make-unique", no_argument, &make_unique_flag, 1},
    // short args
    {"index", required_argument, 0, 'i'},
    {"kmer-size", required_argument, 0, 'k'},
    {0,0,0,0}
  };
  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc,argv,opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 'i': {
      opt.index = optarg;
      break;
    }
    case 'k': {
      stringstream(optarg) >> opt.k;
      break;
    }
    default: break;
    }
  }

  if (verbose_flag) {
    opt.verbose = true;
  }
  if (make_unique_flag) {
    opt.make_unique = true;
  }

  for (int i = optind; i < argc; i++) {
    opt.transfasta.push_back(argv[i]);
  }
}

void ParseOptionsInspect(int argc, char **argv, ProgramOptions& opt) {


  const char *opt_string = "G:g:b:";

  int para_flag = 0;
  static struct option long_options[] = {
    // long args
    {"gfa", required_argument, 0, 'G'},
    {"gtf", required_argument, 0, 'g'},
    {"bed", required_argument, 0, 'b'},
    {"paranoid", no_argument, &para_flag, 1},
    {0,0,0,0}
  };

  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc,argv,opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 'G': {
      opt.gfa = optarg;
      break;
    }
    case 'b': {
      stringstream(optarg) >> opt.bedFile;
      break;
    }
    case 'g': {
      stringstream(optarg) >> opt.gtfFile;
      break;
    }
    default: break;
    }
  }
  opt.index = argv[optind];

  if (para_flag) {
    opt.inspect_thorough = true;
  }
}



void ParseOptionsEM(int argc, char **argv, ProgramOptions& opt) {
  int verbose_flag = 0;
  int plaintext_flag = 0;
  int write_index_flag = 0;
  int single_flag = 0;
  int single_overhang_flag = 0;
  int strand_FR_flag = 0;
  int strand_RF_flag = 0;
  int bias_flag = 0;
  int pbam_flag = 0;
  int gbam_flag = 0;
  int fusion_flag = 0;

  const char *opt_string = "t:i:l:s:o:n:m:d:b:g:c:x:X:";
  static struct option long_options[] = {
    // long args
    {"verbose", no_argument, &verbose_flag, 1},
    {"plaintext", no_argument, &plaintext_flag, 1},
    {"write-index", no_argument, &write_index_flag, 1},
    {"single", no_argument, &single_flag, 1},
    {"single-overhang", no_argument, &single_overhang_flag, 1},
    {"fr-stranded", no_argument, &strand_FR_flag, 1},
    {"rf-stranded", no_argument, &strand_RF_flag, 1},
    {"bias", no_argument, &bias_flag, 1},
    {"pseudobam", no_argument, &pbam_flag, 1},
    {"genomebam", no_argument, &gbam_flag, 1},
    {"fusion", no_argument, &fusion_flag, 1},
    {"seed", required_argument, 0, 'd'},
    // short args
    {"threads", required_argument, 0, 't'},
    {"index", required_argument, 0, 'i'},
    {"fragment-length", required_argument, 0, 'l'},
    {"sd", required_argument, 0, 's'},
    {"min-fragment-length", required_argument, 0, 'x'},
    {"max-fragment-length", required_argument, 0, 'X'},
    {"output-dir", required_argument, 0, 'o'},
    {"iterations", required_argument, 0, 'n'},
    {"min-range", required_argument, 0, 'm'},
    {"bootstrap-samples", required_argument, 0, 'b'},
    {"gtf", required_argument, 0, 'g'},
    {"chromosomes", required_argument, 0, 'c'},
    {0,0,0,0}
  };
  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc,argv,opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 't': {
      stringstream(optarg) >> opt.threads;
      break;
    }
    case 'i': {
      opt.index = optarg;
      break;
    }
    case 'l': {
      stringstream(optarg) >> opt.fld;
      break;
    }
    case 's': {
      stringstream(optarg) >> opt.sd;
      break;
    }
    case 'x':{
      stringstream(optarg) >> opt.min_fl;
      break;
    }
    case 'X':{
      stringstream(optarg) >> opt.max_fl;
      break;
    }
    case 'o': {
      opt.output = optarg;
      break;
    }
    case 'n': {
      stringstream(optarg) >> opt.iterations;
      break;
    }
    case 'm': {
      stringstream(optarg) >> opt.min_range;
    }
    case 'b': {
      stringstream(optarg) >> opt.bootstrap;
      break;
    }
    case 'g': {
      stringstream(optarg) >> opt.gtfFile;
      break;
    }
    case 'c': {
      stringstream(optarg) >> opt.chromFile;
    }
    case 'd': {
      stringstream(optarg) >> opt.seed;
      break;
    }
    default: break;
    }
  }

  // all other arguments are fast[a/q] files to be read
  for (int i = optind; i < argc; i++) {
    opt.files.push_back(argv[i]);
  }

  if (verbose_flag) {
    opt.verbose = true;
  }

  if (plaintext_flag) {
    opt.plaintext = true;
  }

  if (write_index_flag) {
    opt.write_index = true;
  }

  if (single_flag) {
    opt.single_end = true;
  }

  if (single_overhang_flag) {
    opt.single_overhang = true;
  }

  if (strand_FR_flag) {
    opt.strand_specific = true;
    opt.strand = ProgramOptions::StrandType::FR;
  }

  if (strand_RF_flag) {
    opt.strand_specific = true;
    opt.strand = ProgramOptions::StrandType::RF;
  }

  if (bias_flag) {
    opt.bias = true;
  }

  if (pbam_flag) {
    opt.pseudobam = true;
  }

  if (gbam_flag) {
    opt.pseudobam = true;
    opt.genomebam = true;
  }

  if (fusion_flag) {
    opt.fusion = true;
  }
}

void ParseOptionsEMOnly(int argc, char **argv, ProgramOptions& opt) {
  int verbose_flag = 0;
  int plaintext_flag = 0;

  const char *opt_string = "t:s:l:s:o:n:m:d:b:x:X:";
  static struct option long_options[] = {
    // long args
    {"verbose", no_argument, &verbose_flag, 1},
    {"plaintext", no_argument, &plaintext_flag, 1},
    {"seed", required_argument, 0, 'd'},
    // short args
    {"threads", required_argument, 0, 't'},
    {"fragment-length", required_argument, 0, 'l'},
    {"sd", required_argument, 0, 's'},
    {"min-fragment-length", required_argument, 0, 'x'},
    {"max-fragment-length", required_argument, 0, 'X'},
    {"output-dir", required_argument, 0, 'o'},
    {"iterations", required_argument, 0, 'n'},
    {"min-range", required_argument, 0, 'm'},
    {"bootstrap-samples", required_argument, 0, 'b'},
    {0,0,0,0}
  };
  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc,argv,opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 't': {
      stringstream(optarg) >> opt.threads;
      break;
    }
    case 'l': {
      stringstream(optarg) >> opt.fld;
      break;
    }
    case 's': {
      stringstream(optarg) >> opt.sd;
      break;
    }
    case 'x':{
      stringstream(optarg) >> opt.min_fl;
      break;
    }
    case 'X':{
      stringstream(optarg) >> opt.max_fl;
      break;
    }
    case 'o': {
      opt.output = optarg;
      break;
    }
    case 'n': {
      stringstream(optarg) >> opt.iterations;
      break;
    }
    case 'm': {
      stringstream(optarg) >> opt.min_range;
    }
    case 'b': {
      stringstream(optarg) >> opt.bootstrap;
      break;
    }
    case 'd': {
      stringstream(optarg) >> opt.seed;
      break;
    }
    default: break;
    }
  }

  if (verbose_flag) {
    opt.verbose = true;
  }

  if (plaintext_flag) {
    opt.plaintext = true;
  }
}

void ParseOptionsPseudo(int argc, char **argv, ProgramOptions& opt) {
  int verbose_flag = 0;
  int single_flag = 0;
  int strand_flag = 0;
  int pbam_flag = 0;
  int gbam_flag = 0;
  int umi_flag = 0;

  const char *opt_string = "t:i:l:s:o:b:u:x:X:";
  static struct option long_options[] = {
    // long args
    {"verbose", no_argument, &verbose_flag, 1},
    {"single", no_argument, &single_flag, 1},
    //{"strand-specific", no_argument, &strand_flag, 1},
    {"pseudobam", no_argument, &pbam_flag, 1},
    {"umi", no_argument, &umi_flag, 'u'},
    {"batch", required_argument, 0, 'b'},
    // short args
    {"threads", required_argument, 0, 't'},
    {"index", required_argument, 0, 'i'},
    {"fragment-length", required_argument, 0, 'l'},
    {"sd", required_argument, 0, 's'},
    {"min-fragment-length", required_argument, 0, 'x'},
    {"max-fragment-length", required_argument, 0, 'X'},
    {"output-dir", required_argument, 0, 'o'},
    {0,0,0,0}
  };
  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc,argv,opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 't': {
      stringstream(optarg) >> opt.threads;
      break;
    }
    case 'i': {
      opt.index = optarg;
      break;
    }
    case 'l': {
      stringstream(optarg) >> opt.fld;
      break;
    }
    case 's': {
      stringstream(optarg) >> opt.sd;
      break;
    }
    case 'x':{
      stringstream(optarg) >> opt.min_fl;
      break;
    }
    case 'X':{
      stringstream(optarg) >> opt.max_fl;
      break;
    }
    case 'o': {
      opt.output = optarg;
      break;
    }
    case 'b': {
      opt.batch_mode = true;
      opt.batch_file_name = optarg;
      break;
    }
    default: break;
    }
  }

  if (umi_flag) {
    opt.umi = true;
    opt.single_end = true; // UMI implies single end reads
  }

  // all other arguments are fast[a/q] files to be read
  for (int i = optind; i < argc; i++) {
    opt.files.push_back(argv[i]);
  }

  if (verbose_flag) {
    opt.verbose = true;
  }

  if (single_flag) {
    opt.single_end = true;
    opt.single_overhang = true;
  }



  if (strand_flag) {
    opt.strand_specific = true;
  }

  if (pbam_flag) {
    opt.pseudobam = true;
  }
}

void ParseOptionsQuery(int argc, char **argv, ProgramOptions& opt) {
  int verbose_flag = 0;
  int single_flag = 0;
  int strand_flag = 0;
  int pbam_flag = 0;
  int gbam_flag = 0;
  int umi_flag = 0;

  const char *opt_string = "t:i:l:s:o:b:u:x:X:T:";
  static struct option long_options[] = {
    // long args
    {"verbose", no_argument, &verbose_flag, 1},
    {"single", no_argument, &single_flag, 1},
    //{"strand-specific", no_argument, &strand_flag, 1},
    {"pseudobam", no_argument, &pbam_flag, 1},
    {"umi", no_argument, &umi_flag, 'u'},
    {"batch", required_argument, 0, 'b'},
    // short args
    {"threads", required_argument, 0, 't'},
    {"index", required_argument, 0, 'i'},
    {"fragment-length", required_argument, 0, 'l'},
    {"sd", required_argument, 0, 's'},
    {"min-fragment-length", required_argument, 0, 'x'},
    {"max-fragment-length", required_argument, 0, 'X'},
    {"output", required_argument, 0, 'o'},
    {"transID", required_argument, 0, 'T'},
    {0,0,0,0}
  };
  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc,argv,opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 't': {
      stringstream(optarg) >> opt.threads;
      break;
    }
    case 'i': {
      opt.index = optarg;
      break;
    }
    case 'l': {
      stringstream(optarg) >> opt.fld;
      break;
    }
    case 's': {
      stringstream(optarg) >> opt.sd;
      break;
    }
    case 'x':{
      stringstream(optarg) >> opt.min_fl;
      break;
    }
    case 'X':{
      stringstream(optarg) >> opt.max_fl;
      break;
    }
    case 'o': {
      opt.output = optarg;
      break;
    }
    case 'b': {
      opt.batch_mode = true;
      opt.batch_file_name = optarg;
      break;
    }
    case 'T':{
      opt.transID = optarg;
      break;
    }
    default: break;
    }
  }

  if (umi_flag) {
    opt.umi = true;
    opt.single_end = true; // UMI implies single end reads
  }

  // all other arguments are fast[a/q] files to be read
  for (int i = optind; i < argc; i++) {
    opt.files.push_back(argv[i]);
  }

  if (verbose_flag) {
    opt.verbose = true;
  }

  if (single_flag) {
    opt.single_end = true;
    opt.single_overhang = true;
  }



  if (strand_flag) {
    opt.strand_specific = true;
  }

  if (pbam_flag) {
    opt.pseudobam = true;
  }
}

void ParseOptionsH5Dump(int argc, char **argv, ProgramOptions& opt) {
  int peek_flag = 0;
  const char *opt_string = "o:";
  static struct option long_options[] = {
    // long args
    {"peek", no_argument, &peek_flag, 1},
    // short args
    {"output-dir", required_argument, 0, 'o'},
    {0,0,0,0}
  };

  int c;
  int option_index = 0;
  while (true) {
    c = getopt_long(argc, argv, opt_string, long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 'o': {
      opt.output = optarg;
      break;
    }
    default: break;
    }
  }

  // there should only be one thing here...
  for (int i = optind; i < argc; i++) {
    opt.files.push_back(argv[i]);
  }

  if (peek_flag) {
    opt.peek = true;
  }
}

bool CheckOptionsIndex(ProgramOptions& opt) {

  bool ret = true;

  if (opt.k <= 1 || opt.k >= Kmer::MAX_K) {
    cerr << "Error: invalid k-mer length " << opt.k << ", minimum is 3 and  maximum is " << (Kmer::MAX_K -1) << endl;
    ret = false;
  }

  if (opt.k % 2 == 0) {
    cerr << "Error: k needs to be an odd number" << endl;
    ret = false;
  }

  if (opt.transfasta.empty()) {
    cerr << "Error: no FASTA files specified" << endl;
    ret = false;
  } else {

    for (auto& fasta : opt.transfasta) {
      // we want to generate the index, check k, index and transfasta
      struct stat stFileInfo;
      auto intStat = stat(fasta.c_str(), &stFileInfo);
      if (intStat != 0) {
        cerr << "Error: FASTA file not found " << fasta << endl;
        ret = false;
      }
    }
  }

  if (opt.index.empty()) {
    cerr << "Error: need to specify index file" << endl;
    ret = false;
  }

  return ret;
}

bool CheckOptionsEM(ProgramOptions& opt, bool emonly = false) {

  bool ret = true;

  cerr << endl;
  // check for index
  if (!emonly) {
    if (opt.index.empty()) {
      cerr << ERROR_STR << " index file missing" << endl;
      ret = false;
    } else {
      struct stat stFileInfo;
      auto intStat = stat(opt.index.c_str(), &stFileInfo);
      if (intStat != 0) {
        cerr << ERROR_STR << " index file not found " << opt.index << endl;
        ret = false;
      }
    }
  }

  // check for read files
  if (!emonly) {
    if (opt.files.size() == 0) {
      cerr << ERROR_STR << " Missing read files" << endl;
      ret = false;
    } else {
      struct stat stFileInfo;
      for (auto& fn : opt.files) {
        auto intStat = stat(fn.c_str(), &stFileInfo);
        if (intStat != 0) {
          cerr << ERROR_STR << " file not found " << fn << endl;
          ret = false;
        }
      }
    }

    /*
    if (opt.strand_specific && !opt.single_end) {
      cerr << "Error: strand-specific mode requires single end mode" << endl;
      ret = false;
    }*/

    if (!opt.single_end) {
      if (opt.files.size() % 2 != 0) {
        cerr << "Error: paired-end mode requires an even number of input files" << endl
             << "       (use --single for processing single-end reads)" << endl;
        ret = false;
      }
    }
  }

  if ((opt.fld != 0.0 && opt.sd == 0.0) || (opt.sd != 0.0 && opt.fld == 0.0)) {
    cerr << "Error: cannot supply mean/sd without supplying both -l and -s" << endl;
    ret = false;
  }

  if (opt.single_end && (opt.fld == 0.0 || opt.sd == 0.0)) {
    cerr << "Error: fragment length mean and sd must be supplied for single-end reads using -l and -s" << endl;
    ret = false;
  } else if (opt.fld == 0.0 && ret) {
    // In the future, if we have single-end data we should require this
    // argument
    cerr << "[quant] fragment length distribution will be estimated from the data" << endl;
  } else if (ret && opt.fld > 0.0 && opt.sd > 0.0) {
    cerr << "[quant] fragment length distribution is truncated gaussian with mean = " <<
      opt.fld << ", sd = " << opt.sd << endl;
  }

  if (!opt.single_end && (opt.fld > 0.0 && opt.sd > 0.0)) {
    cerr << "[~warn] you specified using a gaussian but have paired end data" << endl;
    cerr << "[~warn] we suggest omitting these parameters and let us estimate the distribution from data" << endl;
  }

  if (opt.fld < 0.0) {
    cerr << "Error: invalid value for mean fragment length " << opt.fld << endl;
    ret = false;
  }

  if (opt.sd < 0.0) {
    cerr << "Error: invalid value for fragment length standard deviation " << opt.sd << endl;
    ret = false;
  }

  if (opt.iterations <= 0) {
    cerr << "Error: invalid number of iterations " << opt.iterations << endl;
    ret = false;
  }

  if (opt.min_range <= 0) {
    cerr << "Error: invalid value for minimum range " << opt.min_range << endl;
    ret = false;
  }

  if (opt.genomebam) {
    if (!opt.gtfFile.empty()) {
      struct stat stFileInfo;
      auto intStat = stat(opt.gtfFile.c_str(), &stFileInfo);
      if (intStat != 0) {
        cerr << "Error: GTF file " << opt.gtfFile << " does not exist" << endl;
        ret = false;
      }
    } else {
      cerr << "Error: need GTF file for genome alignment" << endl;
      ret = false;
    }

    if (!opt.chromFile.empty()) {
      struct stat stFileInfo;
      auto intStat = stat(opt.chromFile.c_str(), &stFileInfo);
      if (intStat != 0) {
        cerr << "Error: Chromosome file not found: " << opt.chromFile << endl;
        ret = false;
      }
    }

  }


  if (opt.output.empty()) {
    cerr << "Error: need to specify output directory " << opt.output << endl;
    ret = false;
  } else {
    struct stat stFileInfo;
    auto intStat = stat(opt.output.c_str(), &stFileInfo);
    if (intStat == 0) {
      // file/dir exits
      if (!S_ISDIR(stFileInfo.st_mode)) {
        cerr << "Error: file " << opt.output << " exists and is not a directory" << endl;
        ret = false;
      } else if (emonly) {
        // check for directory/counts.txt
        struct stat stCountInfo;
        auto intcountstat = stat((opt.output + "/counts.txt" ).c_str(), &stCountInfo);
        if (intcountstat != 0) {
          cerr << "Error: could not find file " << opt.output << "/counts.txt" << endl;
          ret = false;
        }

        // check for directory/index.saved
        struct stat stIndexInfo;
        auto intindexstat = stat((opt.output + "/index.saved").c_str(), &stIndexInfo);
        if (intindexstat != 0) {
          cerr << "Error: could not find index " << opt.output << "/index.saved" << endl;
          ret = false;
        }
        opt.index = (opt.output + "/index.saved");
      }
    } else {
      if (emonly) {
        cerr << "Error: output directory needs to exist, run quant first" << endl;
        ret = false;
      } else {
        // create directory
        if (my_mkdir(opt.output.c_str(), 0777) == -1) {
          cerr << "Error: could not create directory " << opt.output << endl;
          ret = false;
        }
      }
    }
  }

  if (opt.threads <= 0) {
    cerr << "Error: invalid number of threads " << opt.threads << endl;
    ret = false;
  } else {
    unsigned int n = std::thread::hardware_concurrency();
    if (n != 0 && n < opt.threads) {
      cerr << "Warning: you asked for " << opt.threads
           << ", but only " << n << " cores on the machine" << endl;
    }
    if (opt.threads > 1 && opt.pseudobam) {
      //cerr << "Error: pseudobam is not compatible with running on many threads."<< endl;
      //ret = false;
    }
  }

  if (opt.bootstrap < 0) {
    cerr << "Error: number of bootstrap samples must be a non-negative integer." << endl;
    ret = false;
  }

  return ret;
}



bool CheckOptionsPseudo(ProgramOptions& opt) {

  bool ret = true;

  cerr << endl;
  // check for index
  if (opt.index.empty()) {
    cerr << ERROR_STR << " index file missing" << endl;
    ret = false;
  } else {
    struct stat stFileInfo;
    auto intStat = stat(opt.index.c_str(), &stFileInfo);
    if (intStat != 0) {
      cerr << ERROR_STR << " index file not found " << opt.index << endl;
      ret = false;
    }
  }

  // check for read files
  if (!opt.batch_mode) {
    if (opt.umi) {
      cerr << ERROR_STR << " UMI must be run in batch mode, use --batch option" << endl;
      ret = false;
    }

    if (opt.files.size() == 0) {
      cerr << ERROR_STR << " Missing read files" << endl;
      ret = false;
    } else {
      struct stat stFileInfo;
      for (auto& fn : opt.files) {
        auto intStat = stat(fn.c_str(), &stFileInfo);
        if (intStat != 0) {
          cerr << ERROR_STR << " file not found " << fn << endl;
          ret = false;
        }
      }
    }
  } else {
    if (opt.files.size() != 0) {
      cerr << ERROR_STR << " cannot specify batch mode and supply read files" << endl;
      ret = false;
    } else {
      // check for batch files
      if (opt.batch_mode) {
        struct stat stFileInfo;
        auto intstat = stat(opt.batch_file_name.c_str(), &stFileInfo);
        if (intstat != 0) {
          cerr << ERROR_STR << " file not found " << opt.batch_file_name << endl;
          ret = false;
        }
        // open the file, parse and fill the batch_files values
        std::ifstream bfile(opt.batch_file_name);
        std::string line;
        std::string id,f1,f2;
        while (std::getline(bfile,line)) {
          if (line.size() == 0) {
            continue;
          }
          std::stringstream ss(line);
          ss >> id;
          if (id[0] == '#') {
            continue;
          }
          opt.batch_ids.push_back(id);
          if (opt.single_end && !opt.umi) {
            ss >> f1;
            opt.batch_files.push_back({f1});
            intstat = stat(f1.c_str(), &stFileInfo);
            if (intstat != 0) {
              cerr << ERROR_STR << " file not found " << f1 << endl;
              ret = false;
            }
          } else {
            ss >> f1 >> f2;
            if (!opt.umi) {
              opt.batch_files.push_back({f1,f2});
            } else {
              opt.umi_files.push_back(f1);
              opt.batch_files.push_back({f2});
            }
            intstat = stat(f1.c_str(), &stFileInfo);
            if (intstat != 0) {
              cerr << ERROR_STR << " file not found " << f1 << endl;
              ret = false;
            }
            intstat = stat(f2.c_str(), &stFileInfo);
            if (intstat != 0) {
              cerr << ERROR_STR << " file not found " << f2 << endl;
              ret = false;
            }
          }
        }
      }
    }
  }


  /*
  if (opt.strand_specific && !opt.single_end) {
    cerr << "Error: strand-specific mode requires single end mode" << endl;
    ret = false;
  }*/

  if (!opt.single_end) {
    if (opt.files.size() % 2 != 0) {
      cerr << "Error: paired-end mode requires an even number of input files" << endl
           << "       (use --single for processing single-end reads)" << endl;
      ret = false;
    }
  }

  if (opt.umi) {
    opt.single_end = true;
    if (opt.fld != 0.0 || opt.sd != 0.0) {
      cerr << "[~warn] you supplied fragment length information for UMI data which will be ignored" << endl;
    }
  } else {
    if ((opt.fld != 0.0 && opt.sd == 0.0) || (opt.sd != 0.0 && opt.fld == 0.0)) {
      cerr << "Error: cannot supply mean/sd without supplying both -l and -s" << endl;
      ret = false;
    }

    if (opt.single_end && (opt.fld == 0.0 || opt.sd == 0.0)) {
      cerr << "Error: fragment length mean and sd must be supplied for single-end reads using -l and -s" << endl;
      ret = false;
    } else if (opt.fld == 0.0 && ret) {
      // In the future, if we have single-end data we should require this
      // argument
      cerr << "[quant] fragment length distribution will be estimated from the data" << endl;
    } else if (ret && opt.fld > 0.0 && opt.sd > 0.0) {
      cerr << "[quant] fragment length distribution is truncated gaussian with mean = " <<
        opt.fld << ", sd = " << opt.sd << endl;
    }

    if (!opt.single_end && (opt.fld > 0.0 && opt.sd > 0.0)) {
      cerr << "[~warn] you specified using a gaussian but have paired end data" << endl;
      cerr << "[~warn] we suggest omitting these parameters and let us estimate the distribution from data" << endl;
    }
  }

  if (opt.fld < 0.0) {
    cerr << "Error: invalid value for mean fragment length " << opt.fld << endl;
    ret = false;
  }

  if (opt.sd < 0.0) {
    cerr << "Error: invalid value for fragment length standard deviation " << opt.sd << endl;
    ret = false;
  }

  if (opt.output.empty()) {
    cerr << "Error: need to specify output directory " << opt.output << endl;
    ret = false;
  } else {
    struct stat stFileInfo;
    auto intStat = stat(opt.output.c_str(), &stFileInfo);
    if (intStat == 0) {
      // file/dir exits
      if (!S_ISDIR(stFileInfo.st_mode)) {
        cerr << "Error: file " << opt.output << " exists and is not a directory" << endl;
        ret = false;
      }
    } else {
      // create directory
      if (my_mkdir(opt.output.c_str(), 0777) == -1) {
        cerr << "Error: could not create directory " << opt.output << endl;
        ret = false;
      }
    }
  }

  if (opt.threads <= 0) {
    cerr << "Error: invalid number of threads " << opt.threads << endl;
    ret = false;
  } else {
    unsigned int n = std::thread::hardware_concurrency();
    if (n != 0 && n < opt.threads) {
      cerr << "[~warn]  you asked for " << opt.threads
           << ", but only " << n << " cores on the machine" << endl;
    }
    if (opt.threads > 1 && opt.pseudobam) {
      cerr << "Error: pseudobam is not compatible with running on many threads."<< endl;
      ret = false;
    }
  }

  if (opt.pseudobam) {
    cerr << "Error: Pseudobam not supported yet in pseudo mode, use quant mode to obtain BAM files" << endl;
    ret = false;
  }

  return ret;
}

bool CheckOptionsQuery(ProgramOptions& opt) {

  bool ret = true;

  cerr << endl;
  // check for index
  if (opt.index.empty()) {
    cerr << ERROR_STR << " index file missing" << endl;
    ret = false;
  } else {
    struct stat stFileInfo;
    auto intStat = stat(opt.index.c_str(), &stFileInfo);
    if (intStat != 0) {
      cerr << ERROR_STR << " index file not found " << opt.index << endl;
      ret = false;
    }
  }
  if (opt.output.empty()){
    cerr << "Error: output file is not specified." << endl;
    ret = false;
  }
  if (opt.transID.empty()) {
    cerr << "Error: transID is not specified." << endl;
    ret = false;
  }

  return ret;
}


bool CheckOptionsInspect(ProgramOptions& opt) {

  bool ret = true;
  // check for index
  if (opt.index.empty()) {
    cerr << "Error: index file missing" << endl;
    ret = false;
  } else {
    struct stat stFileInfo;
    auto intStat = stat(opt.index.c_str(), &stFileInfo);
    if (intStat != 0) {
      cerr << "Error: index file not found " << opt.index << endl;
      ret = false;
    }
  }

  if (!opt.bedFile.empty() || !opt.gtfFile.empty()) {
    opt.pseudobam = true;
    opt.genomebam = true;
  }

  if (opt.genomebam) {
    struct stat stFileInfo;
    auto intStat = stat(opt.gtfFile.c_str(), &stFileInfo);
    if (intStat != 0) {
      cerr << "Error: GTF file not found " << opt.gtfFile << endl;
      ret = false;
    }

    if (!opt.chromFile.empty()) {
      struct stat stFileInfo;
      auto intStat = stat(opt.chromFile.c_str(), &stFileInfo);
      if (intStat != 0) {
        cerr << "Error: Chromosome file not found " << opt.chromFile << endl;
        ret = false;
      }
    }

    if (opt.bedFile.empty()) {
      opt.bedFile = opt.index + ".bed";
    }
  }

  return ret;
}

bool CheckOptionsH5Dump(ProgramOptions& opt) {
  bool ret = true;
  if (!opt.peek) {
    if ( opt.output.size() == 0) {
      cerr << "Error: You must specify an output directory." << endl;
      ret = false;
    } else {
      struct stat stFileInfo;
      auto intStat = stat(opt.output.c_str(), &stFileInfo);
      if (intStat == 0) {
        // file/dir exits
        if (!S_ISDIR(stFileInfo.st_mode)) {
          cerr << "Error: tried to open " << opt.output << " but another file already exists there" << endl;
          ret = false;
        }
      } else if (my_mkdir(opt.output.c_str(), 0777) == -1) {
        cerr << "Error: could not create directory " << opt.output << endl;
        ret = false;
      }
    }
  } else {
    if (opt.output.size() > 0) {
      cerr << "Error: Cannot specify output directory and '--peek'. Please specify only one." << endl;
      ret = false;
    }
  }

  if (opt.files.size() == 0) {
    cerr << "Error: Missing H5 files" << endl;
    ret = false;
  } else if (opt.files.size() > 1) {
    cerr << "Error: Please specify only one H5 file" << endl;
    ret = false;
  } else {
    struct stat stFileInfo;
    for (auto& fn : opt.files) {
      auto intStat = stat(fn.c_str(), &stFileInfo);
      if (intStat != 0) {
        cerr << "Error: H5 file not found " << fn << endl;
        ret = false;
      }
    }
  }

  return ret;
}

void PrintCite() {
  cout << "When using this program in your research, please cite" << endl << endl
       << "Quantifying full-length circular RNAs in cancer.(2021)" << endl
       << "Ken Hung-On Yu*, Christina Huan Shi*, Bo Wang, Savio Ho-Chit Chow, Grace Tin-Yun Chung, Ke-En Tan, Yat-Yuen Lim, Anna Chi-Man Tsang, Kwok-Wai Lo, Kevin Y. Yip" << endl
       << endl
       << "Please find the citation of kalliso as follows" << endl
       << "  Bray, N. L., Pimentel, H., Melsted, P. & Pachter, L." << endl
       << "  Near-optimal probabilistic RNA-seq quantification, "<< endl
       << "  Nature Biotechnology 34, 525-527(2016), doi:10.1038/nbt.3519" << endl
       << endl;
}

void PrintVersion() {
  cout << "psirc-quant, version " << PSIRC_QUANT_VERSTION <<endl;
  cout << "developed based upon kallisto, version " << 	KALLISTO_VERSION << endl;
}

void usage() {
  cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl << endl
       << "Usage: psirc-quant <CMD> [arguments] .." << endl << endl
       << "Where <CMD> can be one of:" << endl << endl
       << "    index         Builds index "<< endl
       << "    quant         Runs the quantification algorithm " << endl
       << "    pseudo        Runs the pseudoalignment step " << endl
       << "    query         Queries the unitig path of specified transcript" << endl
       << "    h5dump        Converts HDF5-formatted results to plaintext" << endl
       << "    inspect       Inspects and gives information about an index" << endl
       << "    version       Prints version information" << endl
       << "    cite          Prints citation information" << endl << endl
       << "Running psirc-quant <CMD> without arguments prints usage information for <CMD>"<< endl << endl;
}


void usageIndex() {
  cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl
       << "Builds index" << endl << endl
       << "Usage: psirc-quant index [arguments] FASTA-files" << endl << endl
       << "Required argument:" << endl
       << "-i, --index=STRING          Filename for the index to be constructed " << endl << endl
       << "Optional argument:" << endl
       << "-k, --kmer-size=INT         k-mer (odd) length (default: 31, max value: " << (Kmer::MAX_K-1) << ")" << endl
       << "    --make-unique           Replace repeated target names with unique names" << endl
       << endl;

}

void usageh5dump() {
  cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl
       << "Converts HDF5-formatted results to plaintext" << endl << endl
       << "Usage:  psirc-quant h5dump [arguments] abundance.h5" << endl << endl
       << "Required argument:" << endl
       << "-o, --output-dir=STRING       Directory to write output to" << endl << endl;
}

void usageInspect() {
  cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl << endl
       << "Usage: psirc-quant inspect INDEX-file" << endl << endl
       << "Optional arguments:" << endl
       << "-G, --gfa=STRING        Filename for GFA output of T-DBG" << endl
       << "-g, --gtf=STRING        Filename for GTF file" << endl
       << "-b, --bed=STRING        Filename for BED output (default: index + \".bed\")" << endl << endl;

}

void usageEM(bool valid_input = true) {
  if (valid_input) {

  cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl
       << "Computes equivalence classes for reads and quantifies abundances" << endl << endl;
  }
  //      "----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|"
  cout << "Usage: psirc-quant quant [arguments] FASTQ-files" << endl << endl
       << "Required arguments:" << endl
       << "-i, --index=STRING            Filename for the index to be used for" << endl
       << "                              quantification" << endl
       << "-o, --output-dir=STRING       Directory to write output to" << endl << endl
       << "Optional arguments:" << endl
       << "    --bias                    Perform sequence based bias correction" << endl
       << "-b, --bootstrap-samples=INT   Number of bootstrap samples (default: 0)" << endl
       << "    --seed=INT                Seed for the bootstrap sampling (default: 42)" << endl
       << "    --plaintext               Output plaintext instead of HDF5" << endl
       << "    --fusion                  Search for fusions for Pizzly" << endl
       << "    --single                  Quantify single-end reads" << endl
       << "    --single-overhang         Include reads where unobserved rest of fragment is" << endl
       << "                              predicted to lie outside a transcript" << endl
       << "    --fr-stranded             Strand specific reads, first read forward" << endl
       << "    --rf-stranded             Strand specific reads, first read reverse" << endl
       << "-l, --fragment-length=DOUBLE  Estimated average fragment length" << endl
       << "-s, --sd=DOUBLE               Estimated standard deviation of fragment length" << endl
       << "-x, --min-fragment-length     Minimum length of a valid fragment" << endl
       << "-X, --max-fragment-length     Maximum length of a valid fragment" << endl
       << "                              (default: -l, -s values are estimated from paired" << endl
       << "                               end data, but are required when using --single)" << endl
       << "-t, --threads=INT             Number of threads to use (default: 1)" << endl
       << "    --pseudobam               Save pseudoalignments to transcriptome to BAM file" << endl
       << "    --genomebam               Project pseudoalignments to genome sorted BAM file" << endl
       << "-g, --gtf                     GTF file for transcriptome information" << endl
       << "                              (required for --genomebam)" << endl
       << "-c, --chromosomes             Tab separated file with chrosome names and lengths" << endl
       << "                              (optional for --genomebam, but recommended)" << endl;

}

void usagePseudo(bool valid_input = true) {
  if (valid_input) {
    cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl
         << "Computes equivalence classes for reads and quantifies abundances" << endl << endl;
  }

  cout << "Usage: psirc-quant pseudo [arguments] FASTQ-files" << endl << endl
       << "Required arguments:" << endl
       << "-i, --index=STRING            Filename for the index to be used for" << endl
       << "                              pseudoalignment" << endl
       << "-o, --output-dir=STRING       Directory to write output to" << endl << endl
       << "Optional arguments:" << endl
       << "-u  --umi                     First file in pair is a UMI file" << endl
       << "-b  --batch=FILE              Process files listed in FILE" << endl
       << "    --single                  Quantify single-end reads" << endl
       << "-l, --fragment-length=DOUBLE  Estimated average fragment length" << endl
       << "-s, --sd=DOUBLE               Estimated standard deviation of fragment length" << endl
       << "-x, --min-fragment-length     Minimum length of a valid fragment" << endl
       << "-X, --max-fragment-length     Maximum length of a valid fragment" << endl
       << "                              (default: -l, -s values are estimated from paired" << endl
       << "                               end data, but are required when using --single)" << endl
       << "-t, --threads=INT             Number of threads to use (default: 1)" << endl;
//       << "    --pseudobam               Output pseudoalignments in SAM format to stdout" << endl;

}

void usageQuery(bool valid_input = true) {
  if (valid_input) {
    cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl
         << "Queries for the represenation of specified transcript in unitig graph" << endl << endl;
  }

  cout << "Usage: psirc-quant query [arguments]" << endl << endl
       << "Required arguments:" << endl
       << "-i, --index=STRING            Filename for the index to be used for" << endl
       << "                              pseudoalignment" << endl
       << "-T, --transID                 Transcript ID to investigate" << endl
       << "-o, --output=STRING           Output file" << endl;
//       << "    --pseudobam               Output pseudoalignments in SAM format to stdout" << endl;

}

void usageEMOnly() {
  cout << "psirc-quant " << PSIRC_QUANT_VERSTION << endl
       << "Computes equivalence classes for reads and quantifies abundance" << endl << endl
       << "Usage: psirc-quant quant-only [arguments]" << endl << endl
       << "Required argument:" << endl
       << "-o, --output-dir=STRING       Directory to write output to" << endl << endl
       << "Optional arguments:" << endl
       << "-l, --fragment-length=DOUBLE  Estimated fragment length (default: value is estimated from the input data)" << endl
       << "-x, --min-fragment-length     Minimum length of a valid fragment" << endl
       << "-X, --max-fragment-length     Maximum length of a valid fragment" << endl
       << "-b, --bootstrap-samples=INT   Number of bootstrap samples (default: 0)" << endl
       << "    --seed=INT                Seed for the bootstrap sampling (default: 42)" << endl
       << "    --plaintext               Output plaintext instead of HDF5" << endl << endl;
}

std::string argv_to_string(int argc, char *argv[]) {
  std::string res;
  for (int i = 0; i < argc; ++i) {
    res += argv[i];
    if (i + 1 < argc) {
      res += " ";
    }
  }

  return res;
}

std::string get_local_time() {
  time_t rawtime;
  struct tm * timeinfo;

  time( &rawtime );
  timeinfo = localtime( &rawtime );
  std::string ret(asctime(timeinfo));

  // chomp off the newline
  return ret.substr(0, ret.size() - 1);
}

int main(int argc, char *argv[]) {
  std::cout.sync_with_stdio(false);
  setvbuf(stdout, NULL, _IOFBF, 1048576);


  if (argc < 2) {
    usage();
    exit(1);
  } else {
    auto start_time(get_local_time());
    ProgramOptions opt;
    string cmd(argv[1]);
    if (cmd == "version") {
      PrintVersion();
    } else if (cmd == "cite") {
      PrintCite();
    } else if (cmd == "index") {
      cerr << endl;
      if (argc==2) {
        usageIndex();
        return 0;
      }
      ParseOptionsIndex(argc-1,argv+1,opt);
      if (!CheckOptionsIndex(opt)) {
        usageIndex();
        exit(1);
      } else {
        // create an index
        Kmer::set_k(opt.k);
        KmerIndex index(opt);
        index.BuildTranscripts(opt);
        index.write(opt.index);
        //index.write_details(opt.index+".details");
      }
      cerr << endl;
    } else if (cmd == "inspect") {
      if (argc==2) {
        usageInspect();
        return 0;
      }
      ParseOptionsInspect(argc-1, argv+1, opt);
      if (!CheckOptionsInspect(opt)) {
        usageInspect();
        exit(1);
      } else {
        KmerIndex index(opt);
        index.load(opt);
        InspectIndex(index,opt);
      }

    } else if (cmd == "quant") {
      if (argc==2) {
        usageEM();
        return 0;
      }
      ParseOptionsEM(argc-1,argv+1,opt);
      if (!CheckOptionsEM(opt)) {
        cerr << endl;
        usageEM(false);
        exit(1);
      } else {
        // run the em algorithm
        KmerIndex index(opt);
        index.load(opt);
        if (opt.fusion) {
          // need full transcript sequences
          index.loadTranscriptSequences();
        }

        bool guessChromosomes = false;
        Transcriptome model;
        if (opt.genomebam) {
          if (!opt.chromFile.empty()) {
            model.loadChromosomes(opt.chromFile);
          } else {
            guessChromosomes = true;
          }
          model.parseGTF(opt.gtfFile, index, opt, guessChromosomes);
          //model.loadTranscriptome(index, in, opt);
        }


        int num_processed = 0;
        int num_pseudoaligned = 0;
        int num_unique = 0;

        MinCollector collection(index, opt);
        MasterProcessor MP(index, opt, collection, model);
        num_processed = ProcessReads(MP, opt);

        // save modified index for future use
        if (opt.write_index) {
          index.write((opt.output + "/index.saved"), false);
        }

        // if mean FL not provided, estimate
        std::vector<int> fld;
        if (opt.fld == 0.0) {
          fld = collection.flens; // copy
          collection.compute_mean_frag_lens_trunc();
        } else {
          auto mean_fl = (opt.fld > 0.0) ? opt.fld : collection.get_mean_frag_len();
          auto sd_fl = opt.sd;
          collection.init_mean_fl_trunc( mean_fl, sd_fl );
          //fld.resize(MAX_FRAG_LEN,0); // no obersvations
          fld = trunc_gaussian_counts(0, MAX_FRAG_LEN, mean_fl, sd_fl, 10000);

          // for (size_t i = 0; i < collection.mean_fl_trunc.size(); ++i) {
          //   cout << "--- " << i << '\t' << collection.mean_fl_trunc[i] << endl;
          // }
        }

        std::vector<int> preBias(4096,1);
        if (opt.bias) {
          preBias = collection.bias5; // copy
        }

        auto fl_means = get_frag_len_means(index.target_lens_, collection.mean_fl_trunc);

        /*for (int i = 0; i < collection.bias3.size(); i++) {
          std::cout << i << "\t" << collection.bias3[i] << "\t" << collection.bias5[i] << "\n";
          }*/

        EMAlgorithm em(collection.counts, index, collection, fl_means, opt);
        em.run(10000, 50, true, opt.bias);
        // em.run(25000, 20000, true, opt.bias);

        std::string call = argv_to_string(argc, argv);

        H5Writer writer;
        if (!opt.plaintext) {
          writer.init(opt.output + "/abundance.h5", opt.bootstrap, num_processed, fld, preBias, em.post_bias_, 6,
              index.INDEX_VERSION, call, start_time);
          writer.write_main(em, index.target_names_, index.target_lens_);
        }

        for (int i = 0; i < index.num_trans; i++) {
          num_unique += collection.counts[i];
        }
        for (int i = 0; i < collection.counts.size(); i++) {
          num_pseudoaligned += collection.counts[i];
        }

        plaintext_aux(
            opt.output + "/run_info.json",
            std::string(std::to_string(index.num_trans)),
            std::string(std::to_string(opt.bootstrap)),
            std::string(std::to_string(num_processed)),
            std::string(std::to_string(num_pseudoaligned)),
            std::string(std::to_string(num_unique)),
            PSIRC_QUANT_VERSTION,
            std::string(std::to_string(index.INDEX_VERSION)),
            start_time,
            call);

        plaintext_writer(opt.output + "/abundance.tsv", em.target_names_,
            em.alpha_, em.eff_lens_, index.target_lens_);

        if (opt.bootstrap > 0) {
          auto B = opt.bootstrap;
          std::mt19937_64 rand;
          rand.seed( opt.seed );

          std::vector<size_t> seeds;
          for (auto s = 0; s < B; ++s) {
            seeds.push_back( rand() );
          }

          if (opt.threads > 1) {
            auto n_threads = opt.threads;
            if (opt.threads > opt.bootstrap) {
              cerr
                << "[~warn] number of threads (" << opt.threads <<
                ") greater than number of bootstraps" << endl
                << "[~warn] (cont'd) updating threads to number of bootstraps "
                << opt.bootstrap << endl;
              n_threads = opt.bootstrap;
            }

            BootstrapThreadPool pool(opt.threads, seeds, collection.counts, index,
                collection, em.eff_lens_, opt, writer, fl_means);
          } else {
            for (auto b = 0; b < B; ++b) {
              Bootstrap bs(collection.counts, index, collection, em.eff_lens_, seeds[b], fl_means, opt);
              cerr << "[bstrp] running EM for the bootstrap: " << b + 1 << "\r";
              auto res = bs.run_em();

              if (!opt.plaintext) {
                writer.write_bootstrap(res, b);
              } else {
                plaintext_writer(opt.output + "/bs_abundance_" + std::to_string(b) + ".tsv",
                    em.target_names_, res.alpha_, em.eff_lens_, index.target_lens_);
              }
            }
          }

          cerr << endl;
        }

        if (opt.pseudobam) {

          MP.processAln(em, true);
        }

        cerr << endl;
      }
    } else if (cmd == "quant-only") {
      if (argc==2) {
        usageEMOnly();
        return 0;
      }
      ParseOptionsEMOnly(argc-1,argv+1,opt);
      if (!CheckOptionsEM(opt, true)) {
        usageEMOnly();
        exit(1);
      } else {
        // run the em algorithm
        KmerIndex index(opt);
        index.load(opt, false); // skip the k-mer map
        MinCollector collection(index, opt);
        collection.loadCounts(opt);

        std::vector<int> fld;
        // if mean FL not provided, estimate
        if (opt.fld == 0.0) {
          collection.compute_mean_frag_lens_trunc();
          fld = collection.flens;
        } else {
          auto mean_fl = (opt.fld > 0.0) ? opt.fld : collection.get_mean_frag_len();
          auto sd_fl = opt.sd;
          collection.init_mean_fl_trunc( mean_fl, sd_fl );
          //cout << collection.mean_fl_trunc.size() << endl;
          //fld.resize(MAX_FRAG_LEN,0);
          fld = trunc_gaussian_counts(0, MAX_FRAG_LEN, mean_fl, sd_fl, 10000);
          // for (size_t i = 0; i < collection.mean_fl_trunc.size(); ++i) {
          //   cout << "--- " << i << '\t' << collection.mean_fl_trunc[i] << endl;
          // }
        }

        std::vector<int> preBias(4096,1); // default
        if (opt.bias) {
          // fetch the observed bias
          preBias = collection.bias5; // copy
        }

        auto fl_means = get_frag_len_means(index.target_lens_, collection.mean_fl_trunc);

        EMAlgorithm em(collection.counts, index, collection, fl_means, opt);
        em.run(10000, 50, true, opt.bias);

        std::string call = argv_to_string(argc, argv);
        H5Writer writer;

        if (!opt.plaintext) {
          // setting num_processed to 0 because quant-only is for debugging/special ops
          writer.init(opt.output + "/abundance.h5", opt.bootstrap, 0, fld, preBias, em.post_bias_, 6,
              index.INDEX_VERSION, call, start_time);
          writer.write_main(em, index.target_names_, index.target_lens_);
        } else {
          plaintext_aux(
              opt.output + "/run_info.json",
              std::string(std::to_string(index.num_trans)),
              std::string(std::to_string(opt.bootstrap)),
              std::string(std::to_string(0)),
              std::string(std::to_string(0)),
              std::string(std::to_string(0)),
              PSIRC_QUANT_VERSTION,
              std::string(std::to_string(index.INDEX_VERSION)),
              start_time,
              call);

          plaintext_writer(opt.output + "/abundance.tsv", em.target_names_,
              em.alpha_, em.eff_lens_, index.target_lens_);
        }

        if (opt.bootstrap > 0) {
          std::cerr << "Bootstrapping!" << std::endl;
          auto B = opt.bootstrap;
          std::mt19937_64 rand;
          rand.seed( opt.seed );

          std::vector<size_t> seeds;
          for (auto s = 0; s < B; ++s) {
            seeds.push_back( rand() );
          }

          if (opt.threads > 1) {
            auto n_threads = opt.threads;
            if (opt.threads > opt.bootstrap) {
              cerr << "[btstrp] Warning: number of threads (" << opt.threads <<
                ") greater than number of bootstraps." << endl
                << "[btstrp] (cont'd): Updating threads to number of bootstraps "
                << opt.bootstrap << endl;
              n_threads = opt.bootstrap;
            }

            BootstrapThreadPool pool(n_threads, seeds, collection.counts, index,
                collection, em.eff_lens_, opt, writer, fl_means);
          } else {
            for (auto b = 0; b < B; ++b) {
              Bootstrap bs(collection.counts, index, collection, em.eff_lens_, seeds[b], fl_means, opt);
              cerr << "[bstrp] running EM for the bootstrap: " << b + 1 << "\r";
              auto res = bs.run_em();

              if (!opt.plaintext) {
                writer.write_bootstrap(res, b);
              } else {
                plaintext_writer(opt.output + "/bs_abundance_" + std::to_string(b) + ".tsv",
                    em.target_names_, res.alpha_, em.eff_lens_, index.target_lens_);
              }
            }
          }
        }
        cerr << endl;
      }
    } else if (cmd == "pseudo") {
      if (argc==2) {
        usagePseudo();
        return 0;
      }
      ParseOptionsPseudo(argc-1,argv+1,opt);
      if (!CheckOptionsPseudo(opt)) {
        cerr << endl;
        usagePseudo(false);
        exit(1);
      } else {
        // pseudoalign the reads
        KmerIndex index(opt);
        index.load(opt);

        MinCollector collection(index, opt);
        int num_processed = 0;
        int num_pseudoaligned = 0;
        int num_unique = 0;

        Transcriptome model; // empty model
        MasterProcessor MP(index, opt, collection, model);
        std::vector<std::vector<int>> batchCounts;
        if (!opt.batch_mode) {
          num_processed = ProcessReads(MP, opt);
          collection.write((opt.output + "/pseudoalignments"));
        } else {
          num_processed = ProcessBatchReads(index, opt, collection, batchCounts);
          writeBatchMatrix((opt.output + "/matrix"),index, opt.batch_ids,batchCounts);
        }

        std::string call = argv_to_string(argc, argv);


        for (int id = 0; id < batchCounts.size(); id++) {
          const auto &cc = batchCounts[id];
          for (int i = 0; i < index.num_trans; i++) {
            num_unique += cc[i];
          }
          for (int i = 0; i < cc.size(); i++) {
            num_pseudoaligned += cc[i];
          }
        }


        plaintext_aux(
            opt.output + "/run_info.json",
            std::string(std::to_string(index.num_trans)),
            std::string(std::to_string(0)), // no bootstraps in pseudo
            std::string(std::to_string(num_processed)),
            std::string(std::to_string(num_pseudoaligned)),
            std::string(std::to_string(num_unique)),
            PSIRC_QUANT_VERSTION,
            std::string(std::to_string(index.INDEX_VERSION)),
            start_time,
            call);

        cerr << endl;

        if (opt.pseudobam) {
          std::vector<double> fl_means(index.target_lens_.size(),0.0);
          EMAlgorithm em(collection.counts, index, collection, fl_means, opt);
          MP.processAln(em, false);
        }
      }
    } else if (cmd == "query") {
      if (argc==2) {
        usageQuery();
        return 0;
      }
      ParseOptionsQuery(argc-1,argv+1,opt);
      if (!CheckOptionsQuery(opt)) {
        cerr << endl;
        usageQuery(false);
        exit(1);
      } else {
        // pseudoalign the reads
        KmerIndex index(opt);
        index.load(opt);

        MinCollector collection(index, opt);
        int num_processed = 0;
        int num_pseudoaligned = 0;
        int num_unique = 0;

        {//trace path of unitigs to recover transcripts
          int transID = 0;
          while(index.target_names_[transID] != opt.transID){
            transID++;
          }
          assert(transID < index.target_names_.size());
          index.loadTranscriptSequences();
          vector<array<int, 3>> contigsInPath;//<contigID, startPosInTrans, endPosInTrans>
          {
            int i = transID;
            auto& seqs = index.target_seqs_;
            auto& target_names_ = index.target_names_;
            auto& target_isLinear_ = index.target_isLinear_;
            DBGraph& dbGraph = index.dbGraph;
            auto& kmap = index.kmap;
            int k = opt.k;

            int seqlen = seqs[i].size() - (target_isLinear_[i]?(k-1):0); // number of k-mers
            std::string stmp;
            const char *s = seqs[i].c_str();
            ////std::cout << "sequence number " << i << std::endl;
            //std::cout << ">" << target_names_[i] << std::endl;
            //std::cout << seqs[i] << std::endl;
            TransKmerIterator kit(s, seqs[i].length(), target_isLinear_[i]), kit_end;
            for (; kit != kit_end; ++kit) {
              Kmer x = kit->first;
              //std::cout << "position = " << kit->second << ", mapping " << x.toString() << std::endl;
              Kmer xr = x.rep();
              auto search = kmap.find(xr);
              bool forward = (x==xr);
              KmerEntry val = search->second;
              Contig& contig = dbGraph.contigs[val.contig];

              vector<int> trans;
              for(auto t : contig.transcripts){
                trans.push_back(t.trid);
              }

              auto isUnique = [](const std::vector<int>& u) {
                for (int j = 1; j < u.size(); j++) {
                  if (u[j-1] == u[j]) {
                    return false;
                  }
                }
                return true;
              };

              auto unique = [](const std::vector<int>& u) {
                std::vector<int> v;
                v.reserve(u.size());
                v.push_back(u[0]);
                for (int j = 1; j < u.size(); j++) {
                  if (u[j-1] != u[j]) {
                    v.push_back(u[j]);
                  }
                }
                return v;
              };

              std::sort(trans.begin(), trans.end());
              if (!isUnique(trans)){
                std::vector<int> v = unique(trans);
                swap(trans,v);
              }
              contigsInPath.push_back(array<int, 3>({index.ecmapinv[trans], kit->second, kit->second+contig.length}));

              ContigToTranscript info;
              info.trid = i;
              info.pos = kit->second;
              info.sense = (forward == val.isFw());
              int jump = kit->second + contig.length-1;
              //std::cout << "mapped to contig " << val.contig << ", len = " << contig.length <<  ", pos = " << val.getPos() << ", sense = " << info.sense << std::endl;
              // contig.transcripts.push_back(info);
              // debugging
              if (info.sense) {

                if (info.pos == 0) {
                  stmp.append(contig.seq);
                  //std::cout << contig.seq << std::endl;
                } else {
                  stmp.append(contig.seq.substr(k-1));
                  //std::cout << contig.seq.substr(k-1) << std::endl;
                }
              } else {
                std::string r = revcomp(contig.seq);
                if (info.pos == 0) {
                  stmp.append(r);
                  //std::cout << r << std::endl;
                } else {
                  stmp.append(r.substr(k-1));
                  //std::cout << r.substr(k-1) << std::endl;
                }
              }
              //std::cout << stmp << std::endl;
              //std::cout << "covering seq" << std::endl << seqs[i].substr(kit->second, jump-kit->second +k) << std::endl;
              //std::cout << "id = " << tr.trid << ", (" << tr.start << ", " << tr.stop << ")" << std::endl;
              //std::cout << "contig seq" << std::endl;
              // if (forward == val.isFw()) {
              //   //std::cout << contig.seq << std::endl;
              //   assert(contig.seq.substr(tr.start, k-1 + tr.stop-tr.start) == seqs[i].substr(kit->second, jump-kit->second +k) );
              // } else {
              //   //std::cout << revcomp(contig.seq) << std::endl;
              //   assert(revcomp(contig.seq.substr(tr.start, k-1 + tr.stop-tr.start)) == seqs[i].substr(kit->second, jump-kit->second +k));
              // }
              // if (jump == seqlen) {
              //   //std::cout << std::string(k-1+(tr.stop-tr.start)-1,'-') << "^" << std::endl;
              // }

              // // <-- debugging

              kit.jumpTo(jump);
            }
            //if (seqlen > 0) {
            if (seqlen >= k){
              if(target_isLinear_[i] && stmp != seqs[i]){
                std::cout << ">" << target_names_[i] << std::endl
                        << seqs[i] << std::endl
                        << stmp << std::endl;
                assert(false);
              }else if(!target_isLinear_[i] && (stmp != seqs[i] + seqs[i].substr(0, k-1))){
                std::cout << ">" << target_names_[i] << std::endl
                        << seqs[i] << std::endl
                        << stmp << std::endl;
                assert(false);
              }
            }
          }
          ofstream fout;
          // fout.open(opt.output+"/"+opt.transID.substr(opt.transID.rfind(';')+1)+".info");
          fout.open(opt.output);
          // fout<<"Start\tEnd\tEC\t#reads\t#trans"<<endl;
          fout<<"Start\tEnd\tEC\t#trans"<<endl;
          for(auto ele : contigsInPath){
            // fout<<ele[1]<<"\t"<<ele[2]<<"\t"<<ele[0]<<"\t"<<collection.counts[ele[0]]<<"\t"<<index.ecmap[ele[0]].size()<<endl;
            fout<<ele[1]<<"\t"<<ele[2]<<"\t"<<ele[0]<<"\t"<<index.ecmap[ele[0]].size()<<endl;
          }
          fout.close();
        }
      }
    } else if (cmd == "h5dump") {

      if (argc == 2) {
        usageh5dump();
        exit(1);
      }

      ParseOptionsH5Dump(argc-1,argv+1,opt);
      if (!CheckOptionsH5Dump(opt)) {
        usageh5dump();
        exit(1);
      }

      H5Converter h5conv(opt.files[0], opt.output);
      if (!opt.peek) {
        h5conv.write_aux();
        h5conv.convert();
      }
    }  else {
      cerr << "Error: invalid command " << cmd << endl;
      usage();
      exit(1);
    }

  }

  fflush(stdout);

  return 0;
}
