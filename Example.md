  # Bayesian Example![ISP](docs/isp.png)This picture describes the model used by Internet Service Provider to make certain assertions about current state of the system * The system can have access to the Schedule of Maintainance as well as prior 
  probabilities of scheduled and un-scheduled maintainance 
* System is given probabilities of bad Weather.  System has no direct
  knowledge of current Weather but has access to Weather alert. 
  There are statistical probabilities of how often Weather alert is issued in presence of   bad weather * The statistical probability of Internet Congestion is known.
* The system is performing periodic ping of Internet Servers and using 
  observed packet loss to improve its awareness of Internet congestions.  The statistical probability between Internet Congestion and packet drop is provided.* The system has historically calculated statistical probability of 
  Weather Condition, Maintainace and Congestion causing Equipment Damage and  or Temporary outage* The system has historically calculated probility of service calls received
  dueto Equipment Damage and Outage    This model can be used to make many assertions such as:* When Service Call is received (Call=true) and observing values of   Weather Alert (Alert),   Packet Drop detector (Drops) and maintainance schedule (SCHED) this system can determine a probability   of serious access failure (Damage). The ISP policy could be  to send maintance personal if this probability is above certain percentage* Given observed values of   Weather Alert (Alert),   Packet Drop (Drop) and maintainance schedule (Sched) this system can determine the probability of service calls.The company policy could be to have additional call center crewwhen probability of service call is above certain level. ## Implementation[isp.example.cpp](./test1/isp_exampe.cpp)### Define Variables``` c++1   VarDb db;
2   db.AddVar("Sched");   // Scheduled maintainance
3   db.AddVar("Maint");   // Maintainance in progress 
4   db.AddVar("Weather"); // Bad Rain
5   db.AddVar("Alert");   // Weather alert
6   db.AddVar("Cong");    // Internet congestion
7   db.AddVar("Drop");    // Drop packets
8   db.AddVar("Damage");  // internet access failure, require roll a track
9   db.AddVar("Outage");  // Temporary Internet outage
10  db.AddVar("Call");    // Service Call received
```### Create factors- Scheduled Maintainance Patern ("Sched")This is independent variable -- don't have a parent. 
``` c++
1   VarSet vsSched; 
2   vsSched << db["Sched"];
3   std::shared_ptr<Factor> fSched = std::make_shared<Factor>(vsSched, vsSched);
4   fSched->AddInstance(0, 0.96F);  // not scheduled 96% of the time
5   fSched->AddInstance(1, 0.04F);  // scheduled 4 % of the time
```
(This example illustrates a more "chatty" implementation of factor initialization)     1. Empty VarSet is constructed      2. VarSet is populated with variables. One in this case
     3. Factor is constructed using this VarSet. The Factor constructor
          accepts two VarSets:  
          First varset combines all variables in this factor.           Second VarSet are variables in the head of           predicate defining this factor. For single-variable VarSet both          VarSets are same      4. Add probability of Instance=0. 
        Instance 0 defines probability of variable "Sched"=false.      5. Add probability of Instance=1. 
        Instance 1 defines probability of variable "Sched"=true.  The following factor initializations utilize more compact two-line factor initializations.- Weather Factor describes probability of bad weather
 (This and following following factor initializations utilize more compact  factor initialization syntax.)``` c++1   std::shared_ptr<Factor> fWeather(new Factor({db["Weather"]}, { db["Weather"] }));
2   (*fWeather) << 0.995F << fin;  // 99.5% of the time weather is not bad
```    1. Factor is created and initialized with "Weather" as head variable
    2. Factor is loaded with probability of "bad Weather"=false which is 99.5%.
       fin manipulator automatically completes the factor by loading probability 
       "bad Weather"=true to 0.5% to make 100% compliment
- Load the remaining factors
``` c++
   std::shared_ptr<Factor> fCong(new Factor({ db["Cong"] }, { db["Cong"] }));
   (*fCong) << 0.98F << fin;  // 98% of timethere is no congestion

   // Create factor with conditional probability of schedule was set for this maintainance 
   std::shared_ptr<Factor> fSched(new Factor({db["Maint"], db["Sched"]}, {db["Sched"]}));
   (*fSched) << 0.99F << 0.4F << fin;  // 99% no unscheduled maintiannace, 40% sheduled maintainance is not performed

   // Create factor with conditional probabilities of Alert based on Weather Condition  
   std::shared_ptr<Factor> fAlert(new Factor({ db["Weather"], db["Alert"] }, { db["Alert"] }));
   (*fAlert) << 0.998F << 0.3F << fin;  // 99.8% no bad weather will not alert, 30% bad weather will not cause alert

   // Create factor with conditional probabilities of packet drop based on Internet Congestion
   std::shared_ptr<Factor> fDrop(new Factor({ db["Cong"], db["Drop"] }, { db["Drop"] }));
   (*fDrop) << 0.995F << 0.4F << fin;  // 99.5% no congestion pings will notdrop, 40% congestion packet will not drop

   // probability of damage based on weather condition and maintainace 
   std::shared_ptr<Factor> fDamage(new Factor({ db["Weather"], db["Maint"], db["Damage"] }, { db["Damage"] }));
   (*fDamage) << 0.997F << 0.94F << 0.96F << 0.84F << fin;  

   // probability of outage based on weather, maintainance and congestion
   std::shared_ptr<Factor> fOutage(new Factor({ db["Weather"], db["Maint"], db["Cong"], db["Outage"] }, { db["Outage"] }));
   (*fOutage) << 0.99F << 0.92F << 0.9F << 0.88F << 0.8F << 0.82F << 0.85F << 0.78F << fin;

   // probability of service call based on damage and outage
   std::shared_ptr<Factor> fCall(new Factor({ db["Damage"], db["Outage"], db["Call"] }, { db["Call"] }));
   (*fCall) << 0.99F << 0.6F << 0.8F << 0.6F << fin;

```
### Populate FactorSet

``` c++
   fs.AddFactor(fMaint);
   fs.AddFactor(fWeather);
   fs.AddFactor(fCong);
   fs.AddFactor(fSched);
   fs.AddFactor(fAlert);
   fs.AddFactor(fDrop);
   fs.AddFactor(fDamage);
   fs.AddFactor(fOutage);
   fs.AddFactor(fDrop);
   fs.AddFactor(fCall);

```

### Create observed data sample 
Create Clause object containing data input:
1. Maintainance is scheduled
1. Packet drop is not detected
1. Weather Alert is detected
1. Service Callis received 

``` c++
      Clause cSample({ 
            {db["Sched"], true},{ db["Drop"], false },
            { db["Alert"], true }, {db["Call"],true } 
      });

```

### Perform Query -- Find Most Probable explanation of the sample (MPE)

``` c++

1      fs.PruneEdges(cSample);
2      fs.ApplyClause(cSample);
3      fs.MaximizeVar(db.GetVarSet());
4      std::shared_ptr<Factor> res1 = fs.Merge();
5      InstanceId instanceMpe = res1->GetExtendedClause(0);
6      VarSet vsMpe = res1->GetExtendedVarSet();  
7      Clause clMpe(vsMpe, instanceMpe);
```
      1. Optimize VarSet by removing graph edges that are made irrelavant
          by the sample
      2. Apply sample
      3. Find most probable clause
      4. Merge all data in Factorset to obtain probability value of most probable clause 
      5,6,7. Construct Clause for MPE by fetching VarSet and InstanceId of 
         solved MPE sample

### Perform Query -- Find most probable combination of subset of variables  (MAP)
MPE is a special case of MAP where subset of variables includes all variables of the model.
The found MAP most probable combination is not always the same as subset of MPE combination 

``` c++
1      Clause cSample({
2         { db["Sched"], true },{ db["Drop"], false },
3         { db["Alert"], true },{ db["Call"],true }
4      });
5
6      VarSet vsMap({ db["Damage"], db["Outage"] });
7      // all other variables can be eliminated
8      VarSet vsEliminate = fs.GetVarSet()->Substract(vsMap);
9      // find if any variables that can be pruned 
10     VarSet vsPrune = vsMap.Disjuction(cSample.GetVarSet());
11     fs.PruneVars(vsPrune);
12     fs.PruneEdges(cSample.GetVarSet());
13     fs.ApplyClause(cSample);
14     fs.EliminateVar(vsEliminate);
15     fs.MaximizeVar(vsMap);
16     std::shared_ptr<Factor> res1 = fs.Merge();
17     VarSet vsMapRes = res1->GetExtendedVarSet();
18     InstanceId instanceMap = res1->GetExtendedClause(0);
19     Clause clMap(vsMapRes, instanceMap);
 
```
       1-4. Create Sample clause
       6. Construct subset of variables for whome query will be solved
           -- MAP variables
       8. All other variables will be eliminated fromsubset
       10-11. This prunes all variables that can't affect result of query 
       12. Removes edges that are irrelevent due to Sample
       13. Apply sample
       14. Eliminate variables not in a MAP
       15. Find most probable MAP combination
       16. Obtain single Factor containing the result
       17-19. Constract result Clause clMap by packaging map variables, 
            calculated MAP result. Notice that vsMapRes has 
            exactly same contnent as vsMap, but can't gurantee that 
            order of variables will be same, therefore vsMapRes should be 
            used when combined with instanceMap. 


