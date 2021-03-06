#include <iostream>
#include <istream>
#include "fasttext.h"
#include "real.h"
#include <streambuf>
#include <cstring>
#include <map>

#include "real.h"

extern "C" {

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

std::map<std::string, fasttext::FastText*> g_fasttext_model;

void load_model(char *name, char *path) {
	fasttext::FastText *model=new fasttext::FastText();
	model->loadModel(std::string(path));
	g_fasttext_model[std::string(name)]=model;
}

int get_vector(char* name, char *query, float *results) {
  membuf sbuf(query, query + strlen(query));
  std::istream in(&sbuf);
  fasttext::Vector vectorholder(69);
  g_fasttext_model.at(std::string(name))->getSentenceVector(in, vectorholder);

  for(auto i=0; i<vectorholder.size();i++) {
	  results[i] = vectorholder[i];
  }
  return 1;
}

int predict(char* name, char *query, float *prob, char **buf, int *count, int k, int buf_sz) {
  membuf sbuf(query, query + strlen(query));
  std::istream in(&sbuf);

  std::vector<std::pair<fasttext::real, std::string>> predictions;
  try {
		  g_fasttext_model.at(std::string(name))->predictLine(in,predictions, k, 0.0);

		  int i=0;
		  for (auto it = predictions.cbegin(); it != predictions.cend() && i<k; it++) {
		    *(prob+i) = (float)exp(it->first);
		    strncpy(*(buf+i), it->second.c_str(), buf_sz);
			i++;
		  }
		  *count=i;
		  return 0;
  } catch (const std::exception& e) {
		return 1;
  }
}
}