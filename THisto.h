#ifndef THISTO_H
#define THISTO_H 

#include <string> 
#include <map>
#include <vector>

typedef struct 
{
  unsigned int boardIndex; 
  unsigned int dataReceiver;
  unsigned int chipId;
} TChipIndex;


class THisto { 

 private: 
  int           m_ndim;      // Number of dimensions (1 or 2)
  std::string   m_name;      // Histogram name 
  std::string   m_title;     // Histogram title 
  unsigned int  m_dim[2];    // Dimensions 
  double        m_lim[2][2]; // Limits 
  void**        m_histo;     // Histogram 
  unsigned int  m_size;      // Word size 
  double        m_trash;     // Trash bin

 public:   
  THisto ();                                                                                     // Default constructor ("0-Dim histogram")   
  THisto (std::string name, std::string title, unsigned int nbin, double xmin, double xmax);     // Constructor 1D   
  THisto (std::string name, std::string title, unsigned int nbin1, double xmin1, double xmax1,
                                               unsigned int nbin2, double xmin2, double xmax2);  // Constructor 2D   
  THisto (std::string name, std::string title, unsigned int size, unsigned int nbin1, double xmin1, double xmax1,
                                               unsigned int nbin2, double xmin2, double xmax2);  // Constructor 2D   
  THisto (const THisto &h);                                                                      // Copy constructor   
  ~THisto();                                                                                    // Destructor   
  THisto &operator=  (const THisto &h);                            // Assignment operator   
  double operator()  (unsigned int i) const;                       // Bin read access 1d   
  double operator()  (unsigned int i, unsigned int j) const;       // Bin read access 2d   
  void   Set         (unsigned int i, double val);                 // Bin write access 1d
  void   Set         (unsigned int i, unsigned int j, double val); // Bin write access 2d   
  void   Incr        (unsigned int i);
  void   Incr        (unsigned int i, unsigned int j);
  void   Clear       ();                                           // Reset histo - NO MEMORY DISCARD

  //! Getter methods
  std::string GetName ()      const { return m_name; };
  std::string GetTitle()      const { return m_title; };
  int         GetNDim ()      const { return m_ndim; };
  int         GetNBin (int d) const { if (d >=0 && d <= 1) return m_dim[d]; else return 0; };
  double      GetMin  (int d) const { if (d >=0 && d <= 1) return m_lim[d][0]; else return 0; };
  double      GetMax  (int d) const { if (d >=0 && d <= 1) return m_lim[d][1]; else return 0; };
 };


class TScanHisto {
 private:
  std::map<int, THisto> m_histos;
  int                   m_index;
 public:
  TScanHisto () {};                       // Default constructor;
  TScanHisto (const TScanHisto &sh);      // Copy constructor;
  ~TScanHisto();
  double operator()  (TChipIndex index, unsigned int i, unsigned int j) const;       // Bin read access 2d   
  void AddHisto    (TChipIndex index, THisto histo);
  int  GetSize     () {return m_histos.size();};
  int  GetChipList (std::vector <TChipIndex> &chipList);
  void Clear       ();
  void SetIndex    (int aIndex) {m_index = aIndex;};
  int  GetIndex    () const     {return m_index;};
  void Incr        (TChipIndex index, unsigned int i, unsigned int j);
};

 #endif
