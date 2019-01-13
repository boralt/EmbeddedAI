
#include <factor.h>
#include <Factories.h>
#include <json/json.h>
#include <fstream>
#include <gtest/gtest.h>

#define MAX_NIC 4
#define MAX_DEFLECT 10

using namespace bayeslib;

typedef std::map<std::string, std::shared_ptr<Factor> > MapFactors;
static MapFactors m;

// provisioned probabilities
static double gLocalCongProb[4] = { 95., 2., 0.8, 2.2 };
static double gRemoteCongProb[4] = { 98., 1.0, 0.6, 0.4 };
static double gDeflectCongProb[4] = { 96., 2.0, 1.2, 0.8 };

// provisioned penalties
static int gDropPenalty[4] = { 0, 10, 41, 101 };
static int gLatencyPenalty[3] = { 0, 5, 11 };

// network sizing
static int gNumDeflects =2;
static int gNumLocal = 2;
static int gNumRemote = 1;



static std::string VarName(const char *prefix, int deflect, int localLink=-1, int remoteLink=-1)
{
   char sz[20];

   std::string res = prefix;
   snprintf(sz, 20, "-D%d", deflect);
   res += sz;
   if(localLink >= 0)
   {
      snprintf(sz, 20, "-L%d", localLink);
      res += sz;
   }
   
   if(remoteLink >= 0)
   {
      snprintf(sz, 20, "-R%d", remoteLink);
      res += sz;
   }

   return res;
}

static void BuildCongestion(VarDb &db, int numDeflects, int numLocal, int numRemote)
{
   std::string s;

   VarSet vsAllDrops(db);
   
   
   // build local link conjestion
   for(int nLocal = 1; nLocal <= numLocal; nLocal++)
   {
      s = VarName("IngressLinkConj", 0, nLocal);
      db.AddVar(s, {"None", "Low", "Med", "Hi"});
      VarSet vs(db);
      vs << db[s];
      std::shared_ptr<Factor> fConj = std::make_shared<Factor>(vs, db[s]);
      m[s] = fConj;
      
      *fConj.get() <<
         gLocalCongProb[0] <<
         gLocalCongProb[1] <<
         gLocalCongProb[2] <<
         gLocalCongProb[3] << fin;
   }
   
   // build remote link conjestion
   for(int nRemote = 1; nRemote <= numRemote; nRemote++)
   {
      s = VarName("EgressLinkConj", 0, -1, nRemote);
      db.AddVar(s, {"None", "Low", "Med", "Hi"});
      VarSet vs(db);
      vs << db[s];
      std::shared_ptr<Factor> fConj = std::make_shared<Factor>(vs, db[s]);
      m[s] = fConj;
      *fConj.get() << 
         gRemoteCongProb[0] <<
         gRemoteCongProb[1] <<
         gRemoteCongProb[2] <<
         gRemoteCongProb[3] << fin;

   }

   // deflect congestion
   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      s = VarName("DeflectConj", nDeflect);
      db.AddVar(s, {"None", "Low", "Med", "Hi"});
      VarSet vs(db);
      vs << db[s];
      std::shared_ptr<Factor> fConj = std::make_shared<Factor>(vs, db[s]);
      m[s] = fConj;
      *fConj.get() <<
         gDeflectCongProb[0] <<
         gDeflectCongProb[1] <<
         gDeflectCongProb[2] <<
         gDeflectCongProb[3] << fin;

   }

   
   // build detected drop rates factors
   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         for(int nRemote = 1; nRemote <= numRemote; nRemote++)
         {
            s = VarName("Drop", nDeflect, nLocal, nRemote);
            db.AddVar(s, {"None", "Low", "Med", "Hi"});

            // build varset for detected drop deflect
            VarSet vsNew(db);
            vsNew.Add(db[VarName("EgressLinkConj", 0, -1, nRemote)]);
            vsNew.Add(db[VarName("IngressLinkConj", 0, nLocal)]);
            vsNew.Add(db[VarName("DeflectConj", nDeflect)]);
            vsNew.Add(db[s]);


            std::shared_ptr<Factor> fConj = std::make_shared<Factor>(vsNew, db[s]);
            m[s] = fConj;

            // second set of loops applying low level degrade Variables
             // none, low, med ,high

            InstanceId idInstance = 0;
            for (int dropDetected = 0; dropDetected < 4; dropDetected++)
            {
               for(int dropOnDeflect = 0 ; dropOnDeflect < 4; dropOnDeflect++)
               {
                  for(int dropOnRemote =0; dropOnRemote < 4; dropOnRemote++)
                  {
                     for (int dropOnLocal = 0; dropOnLocal < 4; dropOnLocal++)
                     {
                        // conceptual drop rate
                        double valDrop = log(exp(dropOnDeflect) +  exp(dropOnRemote) +
                                             exp(dropOnLocal));

                        double prob = 0;
                        if(dropDetected > (valDrop+3) || dropDetected < (valDrop -3))
                        {
                           prob = 0.01;
                        }
                        else
                        if(dropDetected > (valDrop+2) || dropDetected < (valDrop -2))
                        {
                           prob = 0.05;
                        }
                        else
                        if(dropDetected > (valDrop+1) || dropDetected < (valDrop -1))
                        {
                           prob = 0.1;
                        }
                        else
                        {
                           prob = 0x84;
                        }
                        
                        fConj->AddInstance(idInstance, prob);
                        idInstance++;
                     }
                  }

               }
            }
            fConj->CompleteProbabilities();
            m[s] = fConj;
         }
      }
   }
}


static void BuildLatencies(VarDb &db, int numDeflects, int numLocal, int numRemote)
{
   std::string s;
   // latencies

   
   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nRemote = 1; nRemote <= numRemote; nRemote++)
      {
         s = VarName("LatencyRemote", nDeflect, -1, nRemote);
         db.AddVar(s, {"Low", "Med", "Hi"});
         
         VarSet vs(db);
         vs << db[s];
         std::shared_ptr<Factor> fLat = std::make_shared<Factor>(vs, db[s]);
         m[s] = fLat;
         Factor::FactorLoader fLatLoader(fLat);
         fLatLoader <<
            0.8 <<
            0.1 <<
            0.1 << fin;
         
      }
   }
   

   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         s = VarName("LatencyLocal", nDeflect, nLocal, -1);
         db.AddVar(s, {"Low", "Med", "Hi"});
         
         VarSet vs(db);
         vs << db[s];
         std::shared_ptr<Factor> fLat = std::make_shared<Factor>(vs, db[s]);
         m[s] = fLat;
         Factor::FactorLoader fLatLoader(fLat);
         fLatLoader <<
            0.8 <<
            0.1 <<
            0.1 << fin;
      }
   }

   // measured latencies
   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         for(int nRemote = 1; nRemote <= numRemote; nRemote++)
         {
            s = VarName("Latency", nDeflect, nLocal, nRemote);
            db.AddVar(s, {"Low", "Med", "Hi"});


            VarSet vsNew(db);
            vsNew.Add(db[VarName("LatencyLocal", nDeflect, nLocal, -1)]);
            vsNew.Add(db[VarName("LatencyRemote", nDeflect, -1, nRemote)]);
            vsNew << db[s];
            std::shared_ptr<Factor> fLat = std::make_shared<Factor>(vsNew, db[s]);

            InstanceId idInstance = 0;
            for (int nLatency = 0; nLatency < 3; nLatency++)
            {
               for(int nLatencyRemote =0; nLatencyRemote < 3; nLatencyRemote++)
               {
                  for (int nLatencyLocal = 0; nLatencyLocal < 3; nLatencyLocal++)
                  {
                     double val = 0.;
                     int nExpected = nLatencyLocal + nLatencyRemote;
                     if (nLatency < (nExpected-2) || nLatency > (nExpected+2))
                     {
                        val = 0.02;
                     }
                     else if ((nLatency < (nExpected-1)) || (nLatency > (nExpected+1)))
                     {
                        val = 0.18;
                     }
                     else
                     {
                        val = 0.8;
                     }

                     fLat->AddInstance(idInstance, val);
                     idInstance++;
                  }
               }
            }

            
            m[s] = fLat;
         }
      }
   }


}


static void BuildDecision(VarDb &db, int numDeflects, int numLocal, int numRemote)
{
   //VarSet vsUtility(db);

   //s = VarName("LatencyRem", nDeflect, -1, nRemote);

   std::list<std::string> arDomainList;
   arDomainList.push_back("None");

   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         arDomainList.push_back(VarName("SQ-Local", nDeflect, nLocal));
      }
   }


   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nRemote = 1; nRemote <= numRemote; nRemote++)
      {
         arDomainList.push_back(VarName("SQ-Remote", nDeflect, -1, nRemote));
      }
   }

   for(int nDeflect = 1; nDeflect <= numDeflects; nDeflect++)
   {
      arDomainList.push_back(VarName("Roll", nDeflect));
   }
                                
   db.AddInitVar("Update", arDomainList, VarType_Decision);

   VarSet vsUpdate(db);
   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         for(int nRemote = 1; nRemote <= numRemote; nRemote++)
         {
            std::string s = VarName("Drop", nDeflect, nLocal, nRemote);
            vsUpdate << db[s];
         }
      }
      for (int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         vsUpdate << db[VarName("LatencyLocal", nDeflect, nLocal, -1)];
      }

      for (int nRemote = 1; nRemote <= numRemote; nRemote++)
      {
         vsUpdate << db[VarName("LatencyRemote", nDeflect, -1, nRemote)];
      }

   }

   vsUpdate << db["Update"];
   std::shared_ptr<Factor> fUpdateDecision = std::make_shared<Factor>(vsUpdate, db["Update"]);
   fUpdateDecision->SetFactorType(VarType_Decision);
   m["Update"] = fUpdateDecision;
   printf("Update size is %ld \n", vsUpdate.GetInstances());

   db.AddVar("Utility", VarType_Utility);

   // Build Utility node
   VarSet vsUtility(db);
   for(int nLocal = 1; nLocal <= numLocal; nLocal++)
   {
      std::string s = VarName("IngressLinkConj", 0, nLocal);
      vsUtility << db[s];
   }

   for(int nRemote = 1; nRemote <= numRemote; nRemote++)
   {
      std::string s = VarName("EgressLinkConj", 0, -1, nRemote);
      vsUtility << db[s];
   }

   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      std::string s = VarName("DeflectConj", nDeflect);
      vsUtility << db[s];
   }   

   
   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nRemote = 1; nRemote <= numRemote; nRemote++)
      {
         std::string s = VarName("LatencyRemote", nDeflect, -1, nRemote);
         vsUtility << db[s];
      }
   }
   

   for(int nDeflect=1; nDeflect <= numDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= numLocal; nLocal++)
      {
         std::string s = VarName("LatencyLocal", nDeflect, nLocal, -1);
         vsUtility << db[s];
      }
   }

   vsUtility << db["Update"];
   
   std::shared_ptr<Factor> fUtility = std::make_shared<Factor>(vsUtility, db["Utility"]);
   
   InstanceId utilityInstance = 0;
   InstanceId maxInstance = vsUtility.GetInstances();
   
   printf("Utility size is %d \n", maxInstance);

   for(utilityInstance = 0; utilityInstance < maxInstance; utilityInstance++)
   {
      int arLocalConj[MAX_NIC] = { 0 };
      int arRemoteConj[MAX_NIC] = { 0 };
      int arDeflectConj[MAX_DEFLECT] = { 0 };
      int arLatLocal[MAX_DEFLECT][MAX_NIC] = { 0 };
      int arLatRemote[MAX_DEFLECT][MAX_NIC] = { 0 };

      int nDeflectsAvail = numDeflects;
      int nLocalAvail = numLocal;
      int nRemoteAvail = numRemote;
      
      int n = 0, d = 0;
      InstanceId i = utilityInstance;
      
      for(n=0; n < numLocal; n++)
      {
         arLocalConj[n] = i % 4;
         i /= 4;
      }
      
      for(n=0; n < numRemote; n++)
      {
         arRemoteConj[n] = i % 4;
         i /= 4;
      }

      for(d=0; d  < numDeflects; d++)
      {
         arDeflectConj[d] = i % 4;
         i /= 4;
      }
      
      for(d=0; d < numDeflects; d++)
      {
         for(n = 0; n <= numRemote; n++)
         {
            arLatRemote[d][n] = i % 3;
            i /= 3;
         }
      }
   
      for(d=0; d < numDeflects; d++)
      {
         for(n = 0; n <= numLocal; n++)
         {
            arLatLocal[d][n] = i % 3;
            i /= 3;
         }
      }

      int nDecision = i % arDomainList.size();

      // apply decision to current row
      if (nDecision < numLocal)
      {
         // squelch local link
         n = nDecision;
         arLocalConj[n] = 0;   // erase local congestion
         for(d = 0; d < numDeflects; d++)  // erase local latencies
            arLatLocal[d][n] = 0;

         nLocalAvail--;
      }
      else if(nDecision < (numLocal + numRemote))
      {
         n = nDecision - numLocal;
         arRemoteConj[n] = 0;
         for(d = 0; d < numDeflects; d++)  // erase remote latencies
            arLatRemote[d][n] = 0;

         nRemoteAvail--;
      }
      else if(nDecision < (numLocal + numRemote + numDeflects))
      {
         d = nDecision - numLocal - numRemote;
         arDeflectConj[d] = 0;
         for(n = 0; n < numLocal; n++)  // erase local latencies
            arLatLocal[d][n] = 0;
         for(n = 0; n < numRemote; n++)  // erase local latencies
            arLatRemote[d][n] = 0;
      }
      else
         abort();
      
      // calculate cost for this row
      double cost = -2;

      // penalties for drop

      for(n=0; n < numLocal; n++)
      {
         cost -= gDropPenalty[arLocalConj[n]];
      }
      
      for(n=0; n < numRemote; n++)
      {
         cost -= gDropPenalty[arRemoteConj[n]];;
      }

      for(d=0; d  < numDeflects; d++)
      {
         cost -= gDropPenalty[arDeflectConj[d]];
      }

      // penalty for no traffic
      if (!nLocalAvail || !nRemoteAvail || !nDeflectsAvail)
         cost -= 1000;

      // penalties for latency
      int nWorthLatency = 0;
      
      for(d  = 0; d < numDeflects; d++)
         for(n=0; n < numLocal; n++)
            for (int nRemote=0; nRemote < numRemote; nRemote++)
            {
               if(nWorthLatency < (arLatLocal[d][n] + arLatRemote[d][nRemote]))
                  nWorthLatency = (arLatLocal[d][n] + arLatRemote[d][nRemote]);
            }

      cost -= gLatencyPenalty[nWorthLatency];
      fUtility->AddInstance(utilityInstance, cost);

   }

   fUtility->SetFactorType(VarType_Utility);
   m["Utility"] = fUtility;
}


int TrafficOptimTest()
{
   
   VarDb db;
   FactorSet fs(db);

   BuildCongestion(db, gNumDeflects, gNumLocal, gNumRemote);
   BuildLatencies(db, gNumDeflects, gNumLocal, gNumRemote);
   BuildDecision(db, gNumDeflects, gNumLocal, gNumRemote);

   //std::string sDiag = db.GetJson();
   //printf("\n==Dvn set==\n%s\n", sDiag.c_str());

   std::for_each(std::begin(m), std::end(m), [&fs](auto it) { fs.AddFactor(it.second);});



   // std::string sDiag = fs.GetJson(db);
   // printf("\n==Dvn set==\n%s\n", sDiag.c_str());


   // fs.SetDebugLevel(FactorSet::DebugLevel_Details);

   std::shared_ptr<DecisionBuilderHelper> dh = fs.BuildDecision();
   Clause cl(db);
   
   for (int nDeflect = 1; nDeflect <= gNumDeflects; nDeflect++)
   {
      for(int nLocal = 1; nLocal <= gNumLocal; nLocal++)
      {
         for(int nRemote = 1; nRemote <= gNumRemote; nRemote++)
         {
            
            cl.AddVar(db[VarName("Drop", nDeflect, nLocal, nRemote)], (nDeflect==1)? 3:0);
            cl.AddVar(db[VarName("Latency", nDeflect, nLocal, nRemote)], 2);
         }
      }
   }
   

   ClauseValue res1 = dh->GetDecisions(cl);
   
   printf("Result is here size=%d var=%s\n", res1.GetVarSet().GetSize(), db[res1.GetVarSet().GetFirst()].c_str());


   printf( "Action is %d with score %f", res1[db["Update"]], res1.GetVal());
   return res1[db["Update"]];
}

   

