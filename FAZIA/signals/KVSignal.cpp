//Created by KVClassFactory on Mon Dec 22 15:46:46 2014
//Author: ,,,

#include "KVSignal.h"
#include "KVString.h"
#include "TMath.h"
#include "KVDigitalFilter.h"
#include "KVDataSet.h"
#include "KVEnv.h"
#include "KVDBParameterList.h"

#include "TMatrixD.h"
#include "TMatrixF.h"
#include "TClass.h"

#define LOG2 (double)6.93147180559945286e-01
# define M_PI     3.14159265358979323846  /* pi */

ClassImp(KVSignal)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSignal</h2>
<h4>Base class for FAZIA signal processing</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVSignal::init()
{
   fPSAIsDone = kFALSE;
   fChannel = kUNKDT;
   fYmin = fYmax = 0;
   fAmplitude = 0;
   fRiseTime = 0;
   fIMax = 0;
   fTMax = 0;
   fBaseLine = 0;
   fSigmaBase = 0;

   fChannelWidth = -1;
   fChannelWidthInt = -1;
   fFirstBL = -1;
   fLastBL = -1;
   fTauRC = -1;
   fTrapRiseTime = -1;
   fTrapFlatTop = -1;
   fSemiGaussSigma = -1;
   fWithPoleZeroCorrection = kFALSE;
   fWithInterpolation = kFALSE;
   fMinimumValueForAmplitude = 0;

   //DeduceFromName();
   ResetIndexes();

   SetDefaultValues();
}

void KVSignal::ResetIndexes()
{
   fIndex = -1;
   fBlock = fQuartet = fTelescope = -1;
   fType = fDet = fTelName = fQuartetName = "";
   fFPGAOutputNumbers = 0;
}

KVSignal::KVSignal()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVSignal::KVSignal(const char* name, const char* title) : TGraph()
{
   // Write your code here
   SetNameTitle(name, title);
   init();
}

//________________________________________________________________

KVSignal::KVSignal(const TString& name, const TString& title) : TGraph()
{
   SetNameTitle(name, title);
   init();

}

KVSignal::~KVSignal()
{
   // Destructor
}

KVSignal* KVSignal::ConvertTo(const Char_t* type)
{
   KVSignal* sig = 0;
   TClass* cl = TClass::GetClass(Form("KV%s", type));
   if (cl) {
      sig = (KVSignal*)cl->New();
      sig->SetData(this->GetN(), this->GetX(), this->GetY());
      sig->LoadPSAParameters();
      delete cl;
   }
   return sig;
}


//________________________________________________________________

void KVSignal::Copy(TObject&) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSignal::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   //TGraph::Copy((TGraph&)obj);
   //KVSignal& CastedObj = (KVSignal&)obj;
}

void KVSignal::Set(Int_t n)
{
   fPSAIsDone = kFALSE;
   TGraph::Set(n);
   fAdc.Set(GetN());

}
//________________________________________________________________

void KVSignal::SetData(Int_t nn, Double_t* xx, Double_t* yy)
{
   Set(nn);
   if (nn == 0) {
      Info("SetData", "called with points number=%d", nn);
      return;
   }
   Int_t np = 0;
   fYmin = fYmax = yy[np];
   SetPoint(np, xx[np], yy[np]);
   for (np = 1; np < nn; np += 1) {
      SetPoint(np, xx[np], yy[np]);
      if (yy[np] < fYmin) fYmin = yy[np];
      if (yy[np] > fYmax) fYmax = yy[np];
   }
   SetADCData();
}

void KVSignal::SetADCData()
{

   fChannelWidthInt = fChannelWidth;
   fAdc.Set(GetN());
   for (int ii = 0; ii < GetN(); ii++) fAdc.AddAt(fY[ii], ii);

}

//________________________________________________________________
void KVSignal::DeduceFromName()
{
   //methods used to identify to from which detector/telescope/quartet/block
   //the signal is associated
   //it is assumed that the name of the signal is defined has it is in the raw data
   //generated by fazia_reader which convert raw acquisition file in raw ROOT files

   ResetIndexes();
   KVString tmp = GetName();
   KVString part = "";
   if (tmp.BeginsWith("B")) {
      //FAZIA telescope denomination
      tmp.Begin("-");
      part = tmp.Next();
      part.ReplaceAll("B", "");
      fBlock = part.Atoi();
      part = tmp.Next();
      part.ReplaceAll("Q", "");
      fQuartet = part.Atoi();
      part = tmp.Next();
      part.ReplaceAll("T", "");
      fTelescope = part.Atoi();
      fType = tmp.Next();
      fDet = GetTitle();
      fDet.ToUpper();

      fDetName.Form("%s-T%d-Q%d-B%03d", fDet.Data(), fTelescope, fQuartet, fBlock);
      fTelName.Form("B%03d-Q%d-T%d", fBlock, fQuartet, fTelescope);
      fQuartetName.Form("B%03d-Q%d", fBlock, fQuartet);

      fIndex = 100 * fBlock + 10 * fQuartet + fTelescope;

   } else if (tmp.BeginsWith("RUTH")) {
      //Info("DeduceFromName","Rutherford signal : %s",GetName());
      tmp.Begin("-");
      fTelName = tmp.Next();
      fType = tmp.Next();

      fDet = GetTitle();
      fDet.ToUpper();

      fDetName.Form("%s-%s", fDet.Data(), fTelName.Data());
   } else {
      Info("DeduceFromName", "Other name format that standard FAZIA name : #%s#", GetName());
   }

}

//________________________________________________________________
Double_t KVSignal::GetPSAParameter(const Char_t* parname)
{

   //DeduceFromName has to be called before

   Double_t lval = -1;
   KVString spar;
   spar.Form("%s.%s", GetType(), parname);
   if (gDataSet)  lval = gDataSet->GetDataSetEnv(spar.Data(), 0.0);
   else           lval = gEnv->GetValue(spar.Data(), 0.0);
   return lval;

}

//________________________________________________________________
void KVSignal::UpdatePSAParameter(KVDBParameterList* par)
{
   for (Int_t ii = 0; ii < par->GetParameters()->GetNpar(); ii += 1) {
      TString nameat(par->GetParameters()->GetNameAt(ii));
      if (nameat == "BaseLineLength") SetBaseLineLength(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "ChannelWidth")    SetChannelWidth(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "ShaperRiseTime") SetShaperRiseTime(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "ShaperFlatTop")   SetShaperFlatTop(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "TauRC")        SetTauRC(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "MinimumAmplitude")   SetAmplitudeTriggerValue(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "InterpolatedChannelWidth")    SetInterpolatedChannelWidth(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "Interpolation")   SetInterpolation((par->GetParameters()->GetDoubleValue(ii) == 1));
      else if (nameat == "PZCorrection")    SetPoleZeroCorrection((par->GetParameters()->GetDoubleValue(ii) == 1));
      else {
         if (nameat == "Detector" || nameat == "Signal" || nameat == "RunRange") {

         } else {
            Warning("UpdatePSAParameter", "Not supported PSA parameter : %d %s\n", ii, nameat.Data());
         }
      }
   }
}

//________________________________________________________________
void KVSignal::LoadPSAParameters()
{
   Info("LoadPSAParameters", "To be defined in child class");
}

//________________________________________________________________
void KVSignal::SetDefaultValues()
{
   //To be defined in child class
}

//________________________________________________________________
void KVSignal::TreateSignal()
{
   Info("TreateSignal", "To be defined in child class");
}

//________________________________________________________________
KVPSAResult* KVSignal::GetPSAResult() const
{
   Info("GetPSAResult", "To be defined in child class");
   return 0;
}

//________________________________________________________________
void KVSignal::SetDetectorName(const Char_t* detname)
{
   fDetName = detname;
}

//________________________________________________________________
void KVSignal::SetDetector(const Char_t* det)
{
   fDet = det;
   fDet.ToUpper();
}
//________________________________________________________________
void KVSignal::Print(Option_t*) const
{
   Info("Print", "\nName: %s - Title: %s", GetName(), GetTitle());
   if (fBlock != -1) {
      printf("\tBlock# %d - Quartet# %d - Telescope# %d\n", fBlock, fQuartet, fTelescope);
      printf("\tType: %s - Detecteur: %s\n", fType.Data(), fDet.Data());
   }
   printf("################\nPSA parameters:\n");
   printf("\tBaseLine: length: %lf first: %lf\n", GetBLLength(), GetBLFirst());
   printf("\tChannelWidth: %lf\n", GetChannelWidth());
   printf("\tTauRC: %lf\n", GetTauRC());
   printf("\tShaperRiseTime: %lf\n", GetShaperRiseTime());
   printf("\tShaperFlatTop: %lf\n", GetShaperFlatTop());
   printf("\tWith Interpolation: %d", Int_t(fWithInterpolation));
   if (fWithInterpolation)
      printf(" %1.2lf", GetInterpolatedChannelWidth());
   printf("\n");
   printf("\tWith PoleZero correction: %d\n", Int_t(fWithPoleZeroCorrection));


}

//________________________________________________________________

void KVSignal::ComputeRawAmplitude(void)
{
   Double_t xx, yy;
   Int_t np = 0;
   GetPoint(np++, xx, yy);
   fYmin = fYmax = yy;

   for (np = 1; np < GetN(); np += 1) {
      GetPoint(np, xx, yy);
      if (yy < fYmin) fYmin = yy;
      if (yy > fYmax) fYmax = yy;
   }
}
//________________________________________________________________

Bool_t KVSignal::TestWidth() const
{
   Double_t x0, x1, y0, y1;

   GetPoint(0, x0, y0);
   GetPoint(1, x1, y1);

   Double_t actual_width = x1 - x0;
   return ((actual_width == GetChannelWidth()));
}

//________________________________________________________________

void KVSignal::ChangeChannelWidth(Double_t newwidth)
{
   Double_t xx, yy;
   for (Int_t ii = 0; ii < GetN(); ii += 1) {
      GetPoint(ii, xx, yy);
      SetPoint(ii, ii * newwidth, yy);
   }
}

//________________________________________________________________


Double_t KVSignal::ComputeBaseLine()
{
   //compute mean value of the signal and the rms between
   // limits defined by fFirstBL and fLastBL

   fBaseLine  = FindMedia(fFirstBL, fLastBL);
   fSigmaBase = TMath::Sqrt(FindSigma2(fFirstBL, fLastBL));
   return fBaseLine;
}

Double_t KVSignal::ComputeEndLine()
{
   //same as ComputeBaseLine method but made on the end of the signal
   //in the same length as for the base line

   fEndLine  = FindMedia(GetN() - (fLastBL - fFirstBL), GetN() - 1);
   fSigmaEnd = TMath::Sqrt(FindSigma2(GetN() - (fLastBL - fFirstBL), GetN() - 1));
   return fEndLine;
}

//________________________________________________________________
Bool_t KVSignal::IsFired()
{
   //ComputeBaseLine and ComputeEndLine methods have to be called before

   if (Int_t(fEndLine - fBaseLine) - 1 > TMath::Nint(TMath::Max(fSigmaBase, fSigmaEnd)))
      return kTRUE;
   return kFALSE;
}
//________________________________________________________________

void KVSignal::RemoveBaseLine()
{

   Add(-1.*ComputeBaseLine());
   ApplyModifications();

}

Double_t KVSignal::ComputeAmplitude()
{
   //Compute and return the absolute value of the signal amplitude
   int i_max = 0;
   for (int i = 0; i < fAdc.GetSize(); i++) {
      if (fAdc.At(i_max) <  fAdc.At(i)) i_max = i;
   }
   fIMax = i_max;
   fTMax = fIMax * fChannelWidth;
   fAmplitude = fAdc.At(i_max);
   return fAmplitude;
}

Double_t KVSignal::ComputeRiseTime()
{
   Multiply(-1);
   fAmplitude *= -1;
   Double_t qt70 = FindTzeroCFDCubic(0.7, 3);
   Double_t qt20 = FindTzeroCFDCubic_rev(0.2, qt70, 3);
   Double_t qtrise72 = qt70 - qt20;
   Multiply(-1);
   fAmplitude *= -1;
   fRiseTime = qtrise72;
   return fRiseTime;
}

double KVSignal::FindMedia(double tsta, double tsto)
{
   int n1 = (int)(tsta / fChannelWidth); // Non molto preciso, ma tant'e'...
   int n2 = (int)(tsto / fChannelWidth);

   return FindMedia(n1, n2);
}

double KVSignal::FindMedia(int tsta, int tsto)
{
   // Calcolo della media nel tratto tra tsta e tsto.
   // NOTA: questo ha senso solo se il segnale e' piatto in quella regione!!

   int n1 = (int)(tsta);
   int n2 = (int)(tsto);

   int N = n2 - n1 + 1;
   //// printf("n1=%d, n2=%d, n=%d, fChannelWidth=%e \n",n1, n2, N, fChannelWidth);
   if (n1 < 0 || n1 >= fAdc.GetSize() ||
         n2 < n1 || n2 >= fAdc.GetSize() ||
         N <= 0 || N >= fAdc.GetSize()) {
      printf("--- FSignal::FindMedia:  tsta=%d, tsto=%d ?? (%d)\n", tsta, tsto, fAdc.GetSize());
      return -1E10;//non cambiare, serve a FindSigma2!!
   }
   double media = 0;
   for (int i = n1; i <= n2; i++)
      media += fAdc.At(i);
   media /= N;
   return media;
}

double KVSignal::FindSigma2(double tsta, double tsto)
{
   // Calcolo della varianza nel tratto tra tsta e tsto.
   // NOTA: questo ha senso solo se il segnale e' piatto in quella regione!!

   int n1 = (int)(tsta / fChannelWidth); // Non molto preciso, ma tant'e'...
   int n2 = (int)(tsto / fChannelWidth);

   return FindSigma2(n1, n2);
}

double KVSignal::FindSigma2(int tsta, int tsto)
{
   // Calcolo della varianza nel tratto tra tsta e tsto.
   // NOTA: questo ha senso solo se il segnale e' piatto in quella regione!!

   int n1 = (int)(tsta);
   int n2 = (int)(tsto);

   int N = n2 - n1 + 1;
   double sigma2 = 0;
   double media = FindMedia(tsta, tsto);
   if (media == -1E10) {
      printf("--- FSignal::FindSigma2(double tsta, double tsto) ---: errore nella media\n");
      return -1;
   }

   for (int i = n1; i <= n2; i++)
      sigma2 += (media - fAdc.At(i)) * (media - fAdc.At(i));

   sigma2 /= N - 1;

   return sigma2;
}


void KVSignal::FIR_ApplyTrapezoidal(double trise, double tflat) // trise=sqrt(12)*tausha di CR-RC^4 se tflat=trise/2
{
   if (tflat < 0) tflat = trise / 2.;
   int irise = (int)(1e3 * trise / fChannelWidth);
   int iflat = (int)(1e3 * tflat / fChannelWidth);

//    Info("FIR_ApplyTrapezoidal","irise %d iflat %d chw %lf",irise,iflat, fChannelWidth);

   TArrayF sorig(fAdc);
   float* data  = fAdc.GetArray();
   float* datao = sorig.GetArray();
   int N        = sorig.GetSize();

   // 1
   for (int i = irise; i < N; i++)         data[i] -= datao[i - irise];
   for (int i = irise + iflat; i < N; i++)   data[i] -= datao[i - irise - iflat];
   for (int i = 2 * irise + iflat; i < N; i++) data[i] += datao[i - 2 * irise - iflat];

   // normalizzazione
   double amp = 1e3 * trise / fChannelWidth;
   data[0] /=  amp;
   for (int i = 1; i < N; i++) data[i] = data[i] / amp + data[i - 1];
}


//Double_t KVSignal::GetAmplitude()
//{
//    //Compute and return the absolute value of the signal amplitude
//  int i_max=0;
//  for(int i=0;i<fAdc.GetSize();i++)
//    {
//      if(fAdc.At(i_max) <  fAdc.At(i) ) i_max = i;
//    }
//  fIMax = i_max;
//  fTMax = fIMax*fChannelWidth;
//  fAmplitude = fAdc.At(i_max);
//  return fAmplitude;
//}


Double_t KVSignal::FindTzeroCFDCubic(double level, int Nrecurr)
{
   // recurr=1 means: linear + 1 approx
   // recurr=0 == FindTzeroCFD
   float* data = fAdc.GetArray();
   int NSamples = fAdc.GetSize();
   double fmax = level * fAmplitude;
   /**** 1) ricerca del punto x2 tale che x2 e' il precedente di tcfd ****/
   int x2 = 0;
   for (; x2 < NSamples; x2++) if (data[x2] < fmax) break;
   x2--;

   return CubicInterpolation(data, x2, fmax, Nrecurr);
}

/***************************************************************************************************/
inline unsigned int ReverseBits(unsigned int p_nIndex, unsigned int p_nBits)
{

   unsigned int i, rev;

   for (i = rev = 0; i < p_nBits; i++) {
      rev = (rev << 1) | (p_nIndex & 1);
      p_nIndex >>= 1;
   }

   return rev;

}


/***************************************************************************************************/
void KVSignal::ApplyWindowing(int window_type) // 0: barlett, 1:hanning, 2:hamming, 3: blackman
{
   // vedi pag. 468 oppenheim-shafer
   int N = fAdc.GetSize() - 1;
   switch (window_type) {
      case 0:/*-------------------- BARLETT ----------------------*/
         for (int n = 0; n <= N; n++)
            fAdc.GetArray()[n] *= (n < N / 2 ? 2.*n / (double)N : 2. - 2.*n / (double)N);
         break;
      case 1:/*-------------------- HANNING ----------------------*/
         for (int n = 0; n <= N; n++)
            fAdc.GetArray()[n] *= 0.5 - 0.5 * cos(2 * M_PI * n / (double)N);
         break;
      case 2:/*-------------------- HAmmING ----------------------*/
         for (int n = 0; n <= N; n++)
            fAdc.GetArray()[n] *= 0.54 - 0.46 * cos(2 * M_PI * n / (double)N);
         break;
      case 3:/*-------------------- BLACKMAN --------------------*/
         for (int n = 0; n <= N; n++)
            fAdc.GetArray()[n] *= 0.42 - 0.5 * cos(2 * M_PI * n / (double)N) + 0.08 * cos(4 * M_PI * n / (double)N);
         break;
      default:
         printf("ERROR in %s: windowtype %d not valid!\n", __PRETTY_FUNCTION__, window_type);
   };
}




/***************************************************************************************************/
int KVSignal::FFT(unsigned int p_nSamples, bool p_bInverseTransform,
                  double* p_lpRealIn, double* p_lpImagIn,
                  double* p_lpRealOut, double* p_lpImagOut)
{
   // copiata e adattata da: http://www.codeproject.com/audio/waveInFFT.asp
   // l'unico vettore che puo' essere NULL e' p_lpImagIn

   if (!p_lpRealIn || !p_lpRealOut || !p_lpImagOut) {
      printf("ERROR in %s: NULL vectors!\n", __PRETTY_FUNCTION__);
      return -1;
   }


   unsigned int NumBits;
   unsigned int i, j, k, n;
   unsigned int BlockSize, BlockEnd;

   double angle_numerator = 2.0 * M_PI;
   double tr, ti;

   //     if( !IsPowerOfTwo(p_nSamples) )
   //     {
   //         return;
   //     }
   if (p_nSamples < 2 || p_nSamples & (p_nSamples - 1)) {
      printf("ERROR in %s: %d not a power of two!\n", __PRETTY_FUNCTION__, p_nSamples);
      return -1;
   }

   if (p_bInverseTransform) angle_numerator = -angle_numerator;

   NumBits = 0; // NumberOfBitsNeeded ( p_nSamples );
   for (NumBits = 0; ; NumBits++) {
      if (p_nSamples & (1 << NumBits)) break;
   }

   for (i = 0; i < p_nSamples; i++) {
      j = ReverseBits(i, NumBits);
      p_lpRealOut[j] = p_lpRealIn[i];
      p_lpImagOut[j] = (p_lpImagIn == NULL) ? 0.0 : p_lpImagIn[i];
   }


   BlockEnd = 1;
   for (BlockSize = 2; BlockSize <= p_nSamples; BlockSize <<= 1) {
      double delta_angle = angle_numerator / (double)BlockSize;
      double sm2 = sin(-2 * delta_angle);
      double sm1 = sin(-delta_angle);
      double cm2 = cos(-2 * delta_angle);
      double cm1 = cos(-delta_angle);
      double w = 2 * cm1;
      double ar[3], ai[3];

      for (i = 0; i < p_nSamples; i += BlockSize) {

         ar[2] = cm2;
         ar[1] = cm1;

         ai[2] = sm2;
         ai[1] = sm1;

         for (j = i, n = 0; n < BlockEnd; j++, n++) {

            ar[0] = w * ar[1] - ar[2];
            ar[2] = ar[1];
            ar[1] = ar[0];

            ai[0] = w * ai[1] - ai[2];
            ai[2] = ai[1];
            ai[1] = ai[0];

            k = j + BlockEnd;
            tr = ar[0] * p_lpRealOut[k] - ai[0] * p_lpImagOut[k];
            ti = ar[0] * p_lpImagOut[k] + ai[0] * p_lpRealOut[k];

            p_lpRealOut[k] = p_lpRealOut[j] - tr;
            p_lpImagOut[k] = p_lpImagOut[j] - ti;

            p_lpRealOut[j] += tr;
            p_lpImagOut[j] += ti;

         }
      }

      BlockEnd = BlockSize;

   }


   if (p_bInverseTransform) {
      double denom = (double)p_nSamples;

      for (i = 0; i < p_nSamples; i++) {
         p_lpRealOut[i] /= denom;
         p_lpImagOut[i] /= denom;
      }
   }
   return 0;
}





int KVSignal::FFT(bool p_bInverseTransform, double* p_lpRealOut, double* p_lpImagOut)
{
   // returns the lenght of FFT( power of 2)
   static double* buffer = NULL;
   static int bufflen = 0;
   int NSA = fAdc.GetSize();
   int ibits = (int)ceil(log((double)NSA) / LOG2);
   NSA = 1 << ibits;

   if (buffer != NULL && bufflen < NSA) {
      delete [] buffer;
      buffer = NULL;
   }


   if (buffer == NULL) {
      bufflen = NSA;
      buffer = new double [NSA];
   }
   unsigned int N = fAdc.GetSize();
   float* data = fAdc.GetArray();
   for (unsigned int i = 0; i < N; i++)
      buffer[i] = data[i];
   // 0 padding
   memset(buffer + N, 0, (NSA - N)*sizeof(double));
   int r = FFT(NSA, p_bInverseTransform, buffer, NULL, p_lpRealOut, p_lpImagOut);
   if (r < 0) return r;
   return NSA;
}
TH1* KVSignal::FFT2Histo(int output, TH1* hh)  // 0 modulo, 1 modulo db (normalized), 2, re, 3 im
{
   unsigned int N = fAdc.GetSize();
   double* re = new double [2 * N];
   double* im = new double [2 * N];
   int NFFT = FFT(0, re, im);
   if (NFFT < 0) {
      printf("ERROR in %s: FFT returned %d!\n", __PRETTY_FUNCTION__, NFFT);
      return NULL;
   }
   int NF = NFFT / 2;
   TH1* h = 0;
   if (!hh) h = new TH1F("hfft", "FFT of FSignal", NF, 0, 1. / fChannelWidth * 1000 / 2);
   else h = hh;

   h->SetStats(kFALSE);
   for (int i = 0; i < NF; i++) {
      switch (output) {
         case 0: // modulo
            h->Fill(h->GetBinCenter(i + 1), sqrt(re[i]*re[i] + im[i]*im[i]));
            break;
         case 1: // modulo db
            h->Fill(h->GetBinCenter(i + 1), log(sqrt(re[i]*re[i] + im[i]*im[i])) * 8.68588963806503500e+00); // numero=20./log(10.)
            break;
         case 2:
            h->Fill(h->GetBinCenter(i + 1), re[i]);
            break;
         case 3:
            h->Fill(h->GetBinCenter(i + 1), im[i]);
            break;
         default:
            printf("ERROR in %s: output=%d not valid!!\n", __PRETTY_FUNCTION__, output);
            break;
      }
   }
   h->GetXaxis()->SetTitle("Frequency");
   delete [] re;
   delete [] im;

   if (output != 1) return h;
   /*** normalizzazione a 0 db ****/
   int imax = 0;
   for (int i = 1; i < NF; i++)
      if (h->GetBinContent(i + 1) > h->GetBinContent(imax + 1))
         imax = i;
   double dbmax = h->GetBinContent(imax + 1);
   for (int i = 0; i < NF; i++)
      h->SetBinContent(i + 1, h->GetBinContent(i + 1) - dbmax);
   h->GetYaxis()->SetTitle("Modulo (dB)");
   return h;
}
Double_t KVSignal::CubicInterpolation(float* data, int x2, double fmax, int Nrecurr)
{
   /*
   NOTA: tutti i conti fatti con i tempi in "canali". aggiungero' il *fChannelWidth
   solo nel return.
   */
   /**** 2) normal CFD ***************************************************/
   double xi0 = (fmax - data[x2]) / (data[x2 + 1] - data[x2]);
   if (Nrecurr <= 0) return fChannelWidth * (x2 + xi0);

   /**** 3) approx successive. *******************************************/
   // scrivo il polinomio come a3*xi**3+a2*xi**2+a1*xi+a0
   // dove xi=tcfd-x2 (ovvero 0<xi<1)
   // con maple:
   //                                         3
   //   (1/2 y2 - 1/6 y1 + 1/6 y4 - 1/2 y3) xi
   //
   //                                      2
   //          + (-y2 + 1/2 y3 + 1/2 y1) xi
   //
   //          + (- 1/2 y2 - 1/6 y4 + y3 - 1/3 y1) xi + y2

   double a3 = 0.5 * data[x2] - (1. / 6.) * data[x2 - 1] + (1. / 6.) * data[x2 + 2] - 0.5 * data[x2 + 1];
   double a2 = (-data[x2] + 0.5 * data[x2 + 1] + 0.5 * data[x2 - 1]);
   double a1 = (- 0.5 * data[x2] - 1. / 6. *data[x2 + 2] + data[x2 + 1] - 1. / 3.* data[x2 - 1]);
   double a0 = data[x2];
   double xi = xi0;
   for (int rec = 0; rec < Nrecurr; rec++) {
      xi += (fmax - a0 - a1 * xi - a2 * xi * xi - a3 * xi * xi * xi) / (a1 + 2 * a2 * xi + 3 * a3 * xi * xi);
   }
   return fChannelWidth * (x2 + xi);
}


double KVSignal::GetDataInter(double t)
{
   if (fAdc.GetSize() <= 0) return 1E10;

   int n = (int)(floor(t / fChannelWidth));
   if (n <= 0) return fAdc.At(0);
   if (n > fAdc.GetSize() - 2) return fAdc.At(fAdc.GetSize() - 1);
   if (n * fChannelWidth == t) return fAdc.At(n);
   double y1 = fAdc.At(n);
   double y2 = fAdc.At(n + 1); //quello prima e quello dopo.
   double x1 = fChannelWidth * n;

   return (t - x1) / fChannelWidth * (y2 - y1) + y1;
}

double KVSignal::GetDataInterCubic(double t)
{
   int x2 = (int)(t / fChannelWidth);
   if (x2 < 1 || x2 > fAdc.GetSize() - 2) return GetDataInter(t);
   float* data = fAdc.GetArray();
   /***** CUT & PASTE DA CubicInterpolation *****/

   double a3 = 0.5 * data[x2] - (1. / 6.) * data[x2 - 1] + (1. / 6.) * data[x2 + 2] - 0.5 * data[x2 + 1];
   double a2 = (-data[x2] + 0.5 * data[x2 + 1] + 0.5 * data[x2 - 1]);
   double a1 = (- 0.5 * data[x2] - 1. / 6. *data[x2 + 2] + data[x2 + 1] - 1. / 3.* data[x2 - 1]);
   double a0 = data[x2];
   double xi = (t / fChannelWidth - x2);
   return a3 * xi * xi * xi + a2 * xi * xi + a1 * xi + a0;
}

/***********************************************/
double KVSignal::GetDataCubicSpline(double t)
{
   // see HSIEH S.HOU IEEE Trans. Acoustic Speech, vol. ASSP-26, NO.6, DECEMBER 1978
   Double_t h = fChannelWidth;
   Double_t xk = floor(t / h) * h;
   int xk_index = (int)(t / h);
   Double_t s = (t - xk) / h;

   float* data = fAdc.GetArray();
   int N = fAdc.GetSize();
   int dimensione = 18; //dimensione della matrice dei campioni.!!!!deve essere pari!!!!
   float cm1, cNm1;


   TMatrixD e(dimensione, dimensione);
   TArrayD data_e(dimensione * dimensione);
   for (int k = 0, i = 0; i < dimensione; i++) {
      data_e[k] = 4.;
      if ((k + 1) < pow(dimensione, 2)) data_e[k + 1] = 1.;
      if ((k - 1) > 0)                 data_e[k - 1] = 1.;
      k += dimensione + 1;
   }
   e.SetMatrixArray(data_e.GetArray());
   e *= 1. / 6 / h;
   e.Invert();

   double dati_b[] = { -1, 3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0};
   TMatrixD delta(4, 4, dati_b);


   TMatrixF samples(dimensione, 1);
   TMatrixF coeff(dimensione, 1);
   TMatrixF b(4, 1);
   TMatrixF coefficienti(4, 1);

   if (xk_index < (dimensione / 2 - 1)) {
      samples.SetMatrixArray(data);//prendiamo i dati a partire dal primo
   } else if (xk_index > (N - dimensione / 2 - 1)) {
      samples.SetMatrixArray(data + N - dimensione); //prendiamo 18 dati partendo dalla fine
   } else {
      samples.SetMatrixArray(data + xk_index - (dimensione / 2 - 1)); //perche l'interp deve avvenire tra i campioni centrali della matrice
   }
   float* samples_interp = samples.GetMatrixArray();

   //        coeff=e*samples;
   coeff.Mult(e, samples);
   float* ck = coeff.GetMatrixArray();

   if (xk_index < (dimensione / 2 - 1)) {
      if (xk_index == 0) {
         cm1 = (*samples_interp) * 6 * h - ((*ck) * 4 + (*(ck + 1)));
         float caso_zero[4] = {cm1, *ck, *(ck + 1), *(ck + 2)};
         coefficienti.SetMatrixArray(caso_zero);
      } else coefficienti.SetMatrixArray(ck + xk_index - 1);
   } else if (xk_index > (N - dimensione / 2 - 1)) {
      if (xk_index == N - 2) {
         cNm1 = (*(samples_interp + dimensione - 1)) * 6 * h - (*(ck + dimensione - 1) * 4 + (*(ck + dimensione - 2)));
         float casoN[4] = {*(ck + dimensione - 3), *(ck + dimensione - 2), *(ck + dimensione - 1), cNm1};
         coefficienti.SetMatrixArray(casoN);
      } else coefficienti.SetMatrixArray(ck + dimensione - (N - xk_index + 1));
   } else {
      coefficienti.SetMatrixArray(ck + (dimensione / 2 - 2));
   }

   //   b=delta*coefficienti;
   b.Mult(delta, coefficienti);
   float* b_interp = b.GetMatrixArray();
   float a0 = *b_interp;
   float a1 = *(b_interp + 1);
   float a2 = *(b_interp + 2);
   float a3 = *(b_interp + 3);

   return (1. / 6 / h * (a3 + a2 * s + a1 * s * s + a0 * s * s * s));
}


/***********************************************/
void KVSignal::BuildCubicSplineSignal(double taufinal)
{
   const int Nsa = fAdc.GetSize();
   const double tau = fChannelWidth;

   fChannelWidthInt = taufinal;
   TArrayF interpo;
   interpo.Set((int)(Nsa * tau / taufinal));

   for (int i = 0; i < interpo.GetSize(); i++) interpo.AddAt(GetDataCubicSpline(i * taufinal), i);
   fAdc.Set(0);
   fAdc.Set(interpo.GetSize());
   for (int i = 0; i < interpo.GetSize(); i++) fAdc.AddAt(interpo.At(i), i);

}
/***********************************************/
void KVSignal::BuildCubicSplineSignal()
{
   BuildCubicSplineSignal(GetInterpolatedChannelWidth());
}



/***********************************************/
void KVSignal::BuildCubicSignal(double taufinal)
{
   const int Nsa = fAdc.GetSize();
   const double tau = fChannelWidth;

   fChannelWidthInt = taufinal;
   TArrayF interpo;
   interpo.Set((int)(Nsa * tau / taufinal));
   int nlast = interpo.GetSize() - (int)(3 * tau / taufinal);
   if (nlast <= 0) return;

   for (int i = 0; i < interpo.GetSize(); i++) interpo.AddAt(GetDataInterCubic(i * taufinal), i);
   fAdc.Set(0);
   fAdc.Set(interpo.GetSize());
   for (int i = 0; i < nlast; i++) fAdc.AddAt(interpo.At(i), i);
   for (int i = nlast; i < interpo.GetSize(); i++) fAdc.AddAt(interpo.At(nlast), i);

}
/***********************************************/
void KVSignal::BuildCubicSignal()
{
   BuildCubicSignal(GetInterpolatedChannelWidth());
}

double KVSignal::FindTzeroCFDCubic_rev(double level, double tend, int Nrecurr)
{
   // recurr=1 means: linear + 1 approx
   // recurr=0 == FindTzeroCFD
   /**************************************/
   float* data = fAdc.GetArray();
   int NSamples = fAdc.GetSize();
   double fmax = level * fAmplitude;
   /**** 1) ricerca del punto x2 tale che x2 e' il precedente di tcfd ****/
   int x2 = (int)(tend / fChannelWidth);
   if (x2 <= 0 || x2 >= NSamples) return -1;
   for (; x2 > 0 ; x2--)
      if (data[x2] > fmax)
         break;
   //  x2--;
   return CubicInterpolation(data, x2, fmax, Nrecurr);
}

void KVSignal::FIR_ApplySemigaus(double tau_usec)
{
   FIR_ApplyRCLowPass(tau_usec);
   FIR_ApplyRCLowPass(tau_usec);
   FIR_ApplyRCLowPass(tau_usec);
   FIR_ApplyRCLowPass(tau_usec);

   FIR_ApplyRCHighPass(tau_usec);

   Multiply(1. / (32. / 3.*TMath::Exp(-4.))); // normalizzazione
}


void KVSignal::FIR_ApplyRCLowPass(double time_usec, int reverse)
{
   // f=1/(2*pi*tau) se tau[ns] allora f->[GHz]
   // fsampling=1/fChannelWidth [GHz]
#define DUEPI 6.28318530717958623
   //    printf("fChannelWidth=%f\n",fChannelWidth);
   double f = 1. / (DUEPI * time_usec * 1000.) * fChannelWidth;
   double x = TMath::Exp(-DUEPI * f);
   double a0 = 1 - x;
   double b = x;
   double a = 0;
   //    printf("f=%f, x=%f\n",x,f);
   FIR_ApplyRecursiveFilter(a0, 1, &a, &b, reverse);
}

void KVSignal::FIR_ApplyRCHighPass(double time_usec, int reverse)
{
   // f=1/(2*pi*tau) se tau[ns] allora f->[GHz]
   // fsampling=1/fChannelWidth [GHz]

   //    printf("fChannelWidth=%f\n",fChannelWidth);
   double f = 1. / (DUEPI * time_usec * 1000.) * fChannelWidth;
   double x = TMath::Exp(-DUEPI * f);
   double a0 = (1 + x) / 2.;
   double a1 = -(1 + x) / 2.;
   double b1 = x;
   //    printf("f=%f, x=%f\n",x,f);
   FIR_ApplyRecursiveFilter(a0, 1, &a1, &b1, reverse);
}
#undef DUEPI

void KVSignal::FIR_ApplyRecursiveFilter(double a0, int N, double* a, double* b, int reverse)
{
   // signal will be: y[n]=a0*x[n]+sum a[k] x[k] + sum b[k] y[k]
   int NSamples = fAdc.GetSize();
   double* datay = new double[NSamples];
   float* datax = fAdc.GetArray();
   //    memset(datay, 0, NSamples*sizeof(float)); //azzero l'array.
   /*----------------------------------------------*/
   int i = 0, k = 0;
   switch (reverse) {
      case 0:// direct
         for (i = 0; i < N; i++) { // primo loop su Npunti
            datay[i] = a0 * datax[i];
            for (k = 0; k < i; k++)
               datay[i] += a[k] * datax[i - k - 1] + b[k] * datay[i - k - 1];
         }
         for (i = N; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[i] = a0 * datax[i];
            for (k = 0; k < N; k++)
               datay[i] += a[k] * datax[i - k - 1] + b[k] * datay[i - k - 1];
         }
         break; // end of direct
      case 1: //reverse: cut & paste from direct and NSamples-1-
         for (i = 0; i < N; i++) { // primo loop su Npunti
            datay[NSamples - 1 - i] = a0 * datax[NSamples - 1 - i];
            for (k = 0; k < i; k++)
               datay[NSamples - 1 - i] += a[k] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k] * datay[NSamples - 1 - (i - k - 1)];
         }
         for (i = N; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[NSamples - 1 - i] = a0 * datax[NSamples - 1 - i];
            for (k = 0; k < N; k++)
               datay[NSamples - 1 - i] += a[k] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k] * datay[NSamples - 1 - (i - k - 1)];
         }
         break;
      case -1: // bidirectional
         FIR_ApplyRecursiveFilter(a0, N, a, b, 0);
         FIR_ApplyRecursiveFilter(a0, N, a, b, 1);
         return;
      default:
         printf("ERROR in %s: reverse=%d not supported\n", __PRETTY_FUNCTION__, reverse);
   }// end of reverse switch.
   /*----------------------------------------------*/
   //   void *memcpy(void *dest, const void *src, size_t n);
   /// non con double! memcpy(datax, datay, NSamples*sizeof(float));
   for (int i = 0; i < NSamples; i++)
      datax[i] = (float)datay[i];
   delete [] datay;
}

void KVSignal::FIR_ApplyMovingAverage(int npoints)  // causal moving average
{
   TArrayF sorig = fAdc;
   float* data = fAdc.GetArray();
   float* datao = sorig.GetArray();

   for (int n = npoints; n < GetNSamples(); n++)
      data[n] = data[n - 1] + (datao[n] - datao[n - npoints]) / npoints;
   for (int n = 0; n < npoints; n++)  data[n] = data[npoints]; // first npoints samples are put equal to first good value

}


void KVSignal::PoleZeroSuppression(Double_t tauRC)
{

   KVDigitalFilter deconv_pa1, lp1, int1;
   lp1 = KVDigitalFilter::BuildRCLowPassDeconv(tauRC, fChannelWidth);
   int1 = KVDigitalFilter::BuildIntegrator(fChannelWidth);
   deconv_pa1 = KVDigitalFilter::CombineStagesMany(&lp1, &int1);

   deconv_pa1.ApplyTo(this);
   Multiply(fChannelWidth / tauRC / 1000.);
}

void KVSignal::ApplyModifications(TGraph* newSignal, Int_t nsa)
{
//    Info("ApplyModifications","called with %d",((newSignal==0)?0:1));
   if (!newSignal) newSignal = this;

   Int_t nn = fAdc.GetSize();
   if (nsa > 0 && nsa < nn) nn = nsa;
   if (newSignal->InheritsFrom("KVSignal"))((KVSignal*)newSignal)->SetChannelWidth(fChannelWidthInt);
   for (int ii = 0; ii < nn; ii++) newSignal->SetPoint(ii, ii * fChannelWidthInt, fAdc.At(ii));
}


void KVSignal::Multiply(Double_t fact)
{
   for (int i = 0; i < fAdc.GetSize(); i++) fAdc.AddAt(fAdc.At(i)*fact, i);
}

void KVSignal::Add(Double_t fact)
{
   for (int i = 0; i < fAdc.GetSize(); i++) fAdc.AddAt(fAdc.At(i) + fact, i);
}
