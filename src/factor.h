/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/

#ifndef __FACTOR_H
#define __FACTOR_H


#include <list>
#include <map>
#include <vector>
#include <bitset>
#include <memory>
#include <array>


#include "common.h"
#include <json/json-forwards.h>

/**
@file
@defgroup API EmbeddedAI library API
@brief Classes containing functionality of the Library
*/

/**
@ingroup API
@{
*/

/// namespace bayeslib
/// @brief Namespace for Bayesian APIs
/// @ingroup API
namespace bayeslib
{

   class VarDb;

   ///Type of node on Beysian Graph @ingroup API
   enum VarType
   {
      VarType_Normal,         ///< Regular Beysian node, assigned prior probabilities 
      VarType_Decision,       ///< Decision node on decision network. Binary decision calculated at runtime
      VarType_Utility         ///< Utility node on decision network assigned payoffs based on combination of Decisions and Normal variables
    };

   /// User access interface
   /// Presentation interface is Json based to allow interface to WEB based GUI
   /// @ingroup API
   class UIElem
   {
      public:
         /// Get Type of Object
         virtual std::string GetType() const = 0;

         /// Get JSON presentation of the object
         virtual std::string GetJson(VarDb &db) const = 0;
   };

   /** Define`s a node on a Beysian Graph

   @ingroup API
    */
   class Var : public UIElem
   {
       public:
       /// Constructor
       Var(std::string sName, VarId id, VarType vtype=VarType_Normal);
       
      /// from UIElem
      std::string GetJson(VarDb &db) const override;
      std::string GetType() const override;

	  /// Is it a special Null Variable
      bool IsNull() { return !mId;}
 
	  /// Get Printable name 
      const std::string & GetName() { return mName; }

	  /// Get Id
      VarId GetId() { return mId; }

      /// Get Type of Variable
      /// @return Type of this variable
      VarType GetVarType() const { return mVarType;}

      /// Add state definitionforthis variable
      /// @param sName name of domain state of this variable
      /// @return assigned state value 
      VarState AddState(std::string sName);

      /// Get domain state name 
      /// @param VarState value
      /// @return name of the state
      std::string GetState(VarState state) const;

      /// Get domain state name 
      /// @param  of the state
      /// @return VarState value name 
      VarState GetState(std::string sStateName) const;

      int GetDomainSize() const { return mVarDomainStates.size(); }

       protected:
       std::string mName;
       VarId mId;
       VarType mVarType;
       std::vector<std::string> mVarDomainStates;

   };

   /** Subset of Nodes on a Graph
      Used as utility class throught out the library
      @ingroup API
   */
   class VarSet : public UIElem
   {

   public:
	  /// Constructor for Empty varset
     VarSet();

	  // Copy Constructor
     //VarSet(const VarSet &vs) : mList(vs.mList) {}
	  //VarSet(VarSet &&vs) = default;

	  /// Construct VarSet with single variable.
     /// @param v variable id in varset 
	  VarSet(const VarId v);

	  /// Constuct with initializer list of variable ids
     /// @param initlist {} list of variables
	  VarSet(std::initializer_list<VarId> initlist);

	  /// Default Destructor 
     virtual ~VarSet() = default;

     /// Compare varsets
	  bool operator ==(const VarSet &another) const;

	  /// Add variable to VarSet
     /// @param id variable to add to VarSet
     void Add(VarId id);

     /// Add all variables from another VarSet
     /// @param another VarSet ot merge into this VarSet
	  void Add(const VarSet &another);

	  /// Add variable to VarSet
     VarSet &operator << (VarId id) { Add(id); return *this; }

	  /// Remove variable from VarSet
     /// @param id. Variable to remove from var set. NoOp if this variable is not present in this VarSet
     void Remove(VarId id);

	  /// Merge in another VarsSet
     /// Same as Add(VarSet another)
     void MergeIn(const VarSet &another);

     /// Test is variable member of VarSet
     /// @param id Variable to test
     /// @return true if variable is in VarSet
     bool HasVar(VarId id) const;

	  /// Test is there intersect with another VarSet
     /// @param another VarSet to test  against this VarSet
     /// @return true if any VarId in #another varset is present in this VarSet
     bool HasVar(const VarSet &another) const;
	  
	  /// find if node of vartype exists in this VarSet
     /// @param vdb reference to database of all Variables in the graph
     /// @param vartype VarType to search in this VarSet
     /// @return true if vartype is present 
     bool HasVarType(const VarDb &vdb, VarType vartype) const;

	  /// produce a subsetof varset with vartype variables
     /// @param vdb reference to database of all Variables in the graph
     /// @param vartype VarType to search in this VarSetset of #vartype type
     /// @return VarSet containing variables in this Var
	  VarSet FilterVarSet(const VarDb &vdb, VarType vartype);

	  /// Get Number of variables
     /// @return Number variales is VarSet
      unsigned int GetSize() const;

      /// Number of clauses defined by this Varset
      /// it is normally 2**GetSize()
      /// @return number of istances of Clauses in this VarSet
      InstanceId GetInstances() const;

      /// Get first Var in VarSet
      /// Used to start iterator sequence 
      /// @return VarId of first variable in the VarSet sequence, 0 if VarSet is empty
      VarId GetFirst() const;

	   /// Iterate to nex variable in VarSet
      /// @param VarId in current iteration that was returned from GetFirst() or previous GetNext()
      /// @return VarId of next variable in the VarSet sequence, 0 if #id was last variable in VarSet 
      VarId GetNext(VarId id) const;

	   /// Index of given variable in VarSet list
      /// @return value between 0 > offs  <= GetSize(), offset of variable in this. VarSet
      ///         returns 0 if variableis not present in this VarSet
      int GetOffs(VarId varid) const;

      /// Abbreviated Json output, doesn't resolve VarIds tonames
	   std::string GetJson() const;

      // from UIElem
	   std::string GetJson(VarDb &db) const override;
      std::string GetType() const override;

      /// Test is VarSet is empty
      /// @return true if this VarSet is empty
      bool IsEmpty() const { return mList.empty(); }

      /// Create VarSet which is intersection of two VarSets
      /// @param vs VarSet intersects with this VarSet 
      /// @return result of intersect 
      VarSet Conjuction(const VarSet &vs) const;

      /// Create VarSet which is union of two VarSets
      /// @param vs VarSet will combine with this VarSet
      /// @return result of union between two VarSets
      VarSet Disjuction(const VarSet &vs) const;

      // Create VarSet that includes all variables in this VarSet not present in another VarSet
      /// @param vs VarSet will be substracted from this VarSet
      /// @return result of substraction from this VarSet
      VarSet Substract(const VarSet &vs) const;

   protected:

      class VarOperator
      {
      public:

         VarOperator(VarId id, u8 var_size, InstanceId idMultiplier);

         VarId mId;
         InstanceId mMultiplier;
         int mSize;
      };


      std::list<VarOperator> mList;

	  // optimization mapping of VarId to indexin mList
	  std::array<int, MAX_SET_SIZE> mOffsetMapping; 

   };

   /**
     VarDb contains information of Variables defined in a system
     In maps VarId unique int id to this information
     @ingroup API
   */
   class VarDb
   {
       public:

       /// Constructor
       VarDb();
      
       /// Add variabe to VarDb
       /// @param sName name of variable
       /// @vtype typeof variable
       void AddVar(Var v);

       /// Find if variable exists in VarDb
       /// @param sName name of variable to check
       bool HasVar(std::string sName);

       /// Map Variable name to VarId
       /// @param s name of variable to search
       /// @return VarId if variable found, 0  otherwise
       VarId operator[](const std::string &s);

       /// Map Variable id to name
       /// @param id VarId to search
       /// @return name if variable found,empty otherwise
       std::string operator[](VarId id);

      /// Get Json
      /// @returns Json string with all variables information
      std::string GetJson();

      /// Get Type of the object
      /// @return "VarDb" 
      std::string GetType();

      /// Get all variables in this VarDb
      /// @return VarSet all variable ids in
      VarSet GetVarSet() const;

      /// Map VarId to VarType of variable
      /// @param id VarId of variable
      /// @return VarType of variable
      VarType GetVarType(VarId id) const ;

       protected:
       using VarMap = std::map<std::string, VarId>; 
       VarMap mMap;

       std::vector<Var> mAr;
       std::vector<VarType> mArVarTypes;

   };
            
   /// structure to use to initialize clause
   /// @ingroup API
   struct ClauseInitializer
   {
      VarId varid;    ///< VarId of variable inside the clause 
      VarState nState;    ///< state of variable inside a clause
   };
   
   /// Clause is one instance/occurance for VarSet (subset of domain variables)
   /// it assign true/false value for each variable in VarSet
   /// Internally, the Clause is defined by integral InstanceId value in which  
   /// the states of all Variables in the varset are binary 0 or 1 placed according to
   /// offset inside the VarSet. Therefore numeric value of InstanceId is only significant 
   /// for Clauses constructed from identical VarSet
   /// @ingroup API
   class Clause : public UIElem
   {
       public:
       
       /// Empty constructor. 
       /// Constructs Clause with empty VarSet
       Clause();

       /// Construct Clause from VarSet
       /// the instanceId is assigned to 0 -- all Variables in VarSet are false 
       Clause(const VarSet &vs);

       /// Construct Clause from VarSet and Json representation of values
       /// used internally during Json de-serialization
       /// @param vs VarSet with subset of variables
       /// @param v Json defining values of variables
       Clause(const VarSet &vs, Json::Value &v);

       /// Construct Clause from VarSet and bitvector representation of Variable values
       /// the bitvector is in Domain order of Variables and VarSet independent, so it can 
       /// be used to transfer Variable values between Clauses of different VarSets
       /// @param vs VarSet of variavles in this VarSet
       /// @param clause is a bitset of with values for variables. This bitset is arranged in order of VarIds variables in domain
       ///        and therefore VarSet independent 
       Clause(const VarSet &vs, const std::bitset<MAX_SET_SIZE> &clause);

       /// Construct Clause from VarSet and integral InstanceId 
       /// Since InstanceId is dependent on VarSet order it is usefull to constructing Clauses of same VarSets
       /// @param vs VarSet with Subset of Domain Variables
       /// @param id InstanceId defining values in this VarSet. This value has only significance for Clauses initiated with the 
       ///            same VarSet
       Clause(const VarSet &vs, InstanceId id);

       /// Construct Clause with list of ClauseInitializers
       /// @param initlist initializer list of ClauseInitializers 
       Clause(std::initializer_list<ClauseInitializer> initlist);

       /// Add Variable to the Clause and assign it the value
       /// @param id VarId to variable to add. If variable already in the Clause, just set its value
       /// @param bVal value to set
       void AddVar(VarId id, VarState nVal);

       /// Set value to variable in the set
       /// @param id If variable already in the Clause,  sets its value. Otherwiae NoOp
       /// @param bVal value to set
       void SetVar(VarId id, VarState nVal);

       /// Get value of variable in the clause
       /// @param vid VarId of value to get
       /// @return value of #vid if present in VarSet, false otherwise
       VarState GetVar(VarId vid) const;

       /// Move this clause to next possible instance, wrappped around to 0 (all Variables are set to false)
       /// @return true if wraparound has occured
       bool Incr();

       /// Move this clause to previous possible instance, wrappped around from 0 to 111...1b (all Variables are set to true)
       /// @return true if wraparound has occured
       bool Decr();

       /// Same        /// @param vid VarId of value to get
       /// @return value of #vid if present in VarSet   
	    VarState operator [](VarId vid)  const { return GetVar(vid); }

       /// Get VarSet of the Clause
       /// @return VarSet subset of Domain Variables
       const VarSet &GetVarSet() const 
       {
           return mVarSet;
       }

      // from UIElem
      /// Get Json representation of this Clause
      /// @param db DB of variables in this domain
      /// @return string with Json representatin of this clause
      virtual std::string GetJson(VarDb &)const override;

      /// return "Clause"
      virtual std::string GetType() const override;

      /// Get integral representation of this Clause. This InstanceId
      /// has significance only in context of Recordset base of this Clause
      /// @return InstanceId integral representation of this Clause
      InstanceId GetInstanceId() const { return mInstanceId; }

	  /// remap this clause into another using another varset
     /// @param vs VarSet with set of variables that will be assigned values from this Clause
     /// @return InstanceId integral representation of another Clause based on #vs VarSet
	  InstanceId GetInstanceId(const VarSet &vs) const;

      // static utility functions
      static Clause Append(VarSet target, const Clause &cl1, const Clause &cl2);
    


       protected:
       void UpdateClause();

       // std::bitset<MAX_SET_SIZE> mClause;
       InstanceId mInstanceId;
       VarSet mVarSet; 

   };

   /// Combination od Clause and ValueType assigned to the  Clause
   /// It conceptually represents entire row in a Factor
   /// @ingroup API
   class ClauseValue : public Clause
   {
   public:

      /// Template constructor uses any of VarSet constructors and assignes value 0f 0.0F
	   template<typename ...Args>  ClauseValue(Args && ... args) :
		   Clause(std::forward<Args>(args) ...), mVal(0)
	   {

	   }

      /// Constructs ClauseValue using Clause and ValueType
      /// @param cl Clause of this ClauseValue
      /// @param v Value that will be assigned to this clause
	   ClauseValue(Clause cl, ValueType v) :
		Clause(cl), mVal(v)
	   {

	   }

      /// Set valuse associated to this clause
      /// @param v Value to assign to this clause
	   void SetVal(ValueType v)
	   {
		   mVal = v;
	   }

      /// Get Value assigned to this CaluseVal
      /// @return value assigned to this ClauseVal
	   ValueType GetVal() const 
	   {
		   return mVal;
	   }

   protected:
	   ValueType mVal;

   };


   class FactorExtender
   {
   public:
      virtual void AddVar(VarId id, bool val) =0;
      

   };

   /// Represents prior probablities or other Value for a node in the Bayesian network
   /// it is a full table of all Clauses possible in the VarSet and values 
   /// assigned to each clause.
   /// @ingroup API
   class
   Factor : public UIElem, public std::enable_shared_from_this<Factor>
   {
    public:

      /// Construct from VarSet, initialized with 0.0 assigned to every row in the table.
      /// @param varset full VarSet of this Factor. Includes both head and tail of the Factor
      ///        For example factor X | A,B,C, describing probabilities of X based on values assigned to A, B and C
      ///        will be using factor A,B,C,X
      Factor(const VarSet &varset);

      /// Construct Factor from VarSet and also specifies the VarId Head part of VarSet
      /// @param varset full VarSet of this Factor. Includes both head and tail of the Factor
      //  @param clauseHead VarId to go into Head part of Facectoe
      Factor(const VarSet &varset, VarId clauseHead);

      /// Construct Factor from VarSet and also specifies the VarSet Head part of VarSet
      /// @param varset full VarSet of this Factor. Includes both head and tail of the Factor
      //  @param clauseHead VarSet to go into Head part of Facectoe
      Factor(const VarSet &varset, VarSet clauseHead);
      

      // perfect forwarding constructor
      // @param varset initializer list for full varset for this Factor
      // @param clauseHead VarSet for Head of this Factor
      Factor(std::initializer_list<VarId> varset, std::initializer_list<VarId> clauseHead);

      /// Set Value to any row (Clause) in a table
      /// @param instance InstanceId representing Clause (row) in this table
      /// @param val ValueType assigned to this row
      void AddInstance(InstanceId instance, ValueType val);

      /// Associate Clause of ExtendedVarSet to a row of this factor. Each row in a Factor can be optionaly associated with Instance (Clause) of another VarSet
      /// called ExtendedVarSet. 
      /// @param instance InstanceId (row) of VarSet of this Factor
      /// @param extendedInstance InstanceId of Clause in ExtendedVarset that is associated with #instance 
      void AddExtendedClause(InstanceId instance, InstanceId extendedInstance);

      /// Get InstanceId of ExtendedVarSet that is associated with a row in a Factor
      /// @param instance InstanceId representing Clause (row) in this table
      /// @return extendedInstance InstanceId of Clause in ExtendedVarset that is associated with #instance 
      InstanceId GetExtendedClause(InstanceId instance);

      /// Erase all ExtendedInfo from this Factor
      void EraseExtendedInfo();

      /// Set VarType of Factor. Normally type of Factor is defined by type of single Variable in Head FactorSet
      /// @param enVarType VarType to assign to this Factor
 	   void SetFactorType(VarType enVarType) { mFactorType = enVarType; }

      /// Get VarType type assigned to this Factor
      /// @return VarType assigned to this Factor
	   VarType GetFactorType() const { return mFactorType;  }

      /// Modify ExtendedVarSet assigned to this Factor
      /// ExtendedVarSet is a VarSet that is associated with this Factor that is different from VarSet set of variables
      /// defining this factor
      /// @param vs VarSet that will be assigned to ExtendedVarSet
      void SetExtendedVarSet(const VarSet& vs)
      {
         mExtendedVarSet = vs;
      }

      /// Get ExtendedVarSet assigned to this Factor
      /// ExtendedVarSet is a VarSet that is associated with this Factor that is different from VarSet set of variables
      /// defining this factor
      /// @return VarSet that is assigned to ExtendedVarSet
      /// @ingroup API
      const VarSet & GetExtendedVarSet() const
      {
         return mExtendedVarSet;
      }

      /// Get VarSet that this Factor is based on
      /// @return VarSet that this Factor is based on 
      const VarSet & GetVarSet() const { return mSet;}

      /// Determine if VarValue was explicitly assigent to specific instance using AddInstance call
      /// @param id InstanceId (row) in Factor to check
      /// @return true if this InstanceId was previously assgned value
      bool HasVal(InstanceId id);

      /// Get Tail part of VarSet that defines this Factor. It is full VarSet subtracted head Factor 
      /// @return Tail VarSet
      VarSet GetVarSetTail() const
      {
         return mSet.Substract(mClauseHead);
      }

      /// Configure Head VarSet of this Factor. In most cases it is subset of full VarSet
      /// @param vs VarSet that will be configured as Head VarSet
      void SetClauseHead(const VarSet &vs)
      {
          mClauseHead = vs;
      }

      /// Get Head VarSet of this factor
      /// @return Head VarSet 
      const VarSet &GetClauseHead() const
      {
          return mClauseHead;
      }

      /// Check if this factor is based on empty VarSet 
      /// @return true if this Factor is based on empty VarSet
      bool IsEmpty() const { return mSet.IsEmpty(); } 

      /// Produce a Factor which is a result of selecting rows from this factor
      /// that are matching Clause. Clause should be based on VarSet subset of this Factor's VarSet
      /// @parameter c Clause to apply
      /// @return Factor with VarSet that is result of this Factor's VarSet minus Clause VarSet containing
      ///         only parts of the raws that matched intersection of Factor's VarSet and Clause's VarSet
      std::shared_ptr<Factor> ApplyClause(const Clause &c);

      /// Produce a Factor that is a merge of this Factor and another Factor
      /// @param f factor to merge with this Factor
      /// @return Factor that  is combination ow two factors. It's VarSet is a union of two Factor's VarSets
      std::shared_ptr<Factor> Merge(std::shared_ptr<Factor> f); 

      /// Eliminate variable from Factor by summing up the Instances of Clauses that 
      /// differentiate by this VarId only
      /// @param id VarId that will be eliminated from Factor by summing rows
      /// @return Factor with eliminated Variable
      std::shared_ptr<Factor> EliminateVar(VarId id);

      /// Eliminate set of variables from Factor by summing up the Instances of Clauses that 
      /// differentiate by these Variables
      /// @param vs VaraSet that will be eliminated from Factor by summing rows
      /// @return Factor with eliminated Variables
      std::shared_ptr<Factor> EliminateVar(const VarSet &vs);
      
      /// Eliminate Variable from Factor by grouping rows that different by
      /// a passed Variable only and selectin one that have maximum value
      /// @param id VarId of variable to eliminate
      /// @return Factor with eliminated Variable
      std::shared_ptr<Factor> MaximizeVar(VarId id);

      /// Modify values assigned to the rows (Clauses) so to insure
      /// that values match rules of statistical math.
      /// @return Factor with normalized row values
      std::shared_ptr<Factor> Normalize();

	  /// Complete probabilities table mValues to make conditional 
	  /// probabilities pairs to 100%. This is done by filling up values in non-present
     /// rows based on values in present raws
	  void CompleteProbabilities();

     /// removes Variable from factor by removing all rows that not matching state 
     /// of the Variable
     /// @param v VarId of variable to eliminate
     /// @param val value of variable that matches rows that will be retained in returned VarSet
     /// @return Factor with eliminated variable
     std::shared_ptr<Factor> PruneEdge(VarId v, bool val);

      // from UIElem
      /// produce Json representation of the Factor
      /// @param VarDb of domain variables
      /// @return string with Json representation of VarSet
      virtual std::string GetJson(VarDb &db) const override;

      /// returns "Factor"
      virtual std::string GetType() const override;

      /// Get Value assigned to a row (InstenceId)
      /// @param id InstanceId of rowfactor
      /// @return Value assigned to this row in 
      ValueType Get(InstanceId id);
    
      void SetFactorExtender(FactorExtender *p)
      {
         mpExtender = p;
      }
      FactorExtender *GetFactorvgcxExtender() { return mpExtender; }

	  using FactorLoaderManipulator = void(*)(std::shared_ptr<Factor>);

     /// Utility class to populate Factor with Values
     /// @ingroup API
      class FactorLoader
      {
      public:
         /// Construct FactorLoader with Factor
         /// @param f Factor that will be loaded using FactorLoader 
         FactorLoader(std::shared_ptr<Factor> f);

         /// Load next Instance (row) in Factor with Value
         /// @param v ValueType value to load
         /// @return reference to this FactorLoader to facilitate chain operations
         FactorLoader &operator <<(ValueType v);

         /// Inject special FactorLoader manipulator to perform operation during loading
         /// The only manipulator that is currently available is #fin manipulator
         FactorLoader &operator <<(FactorLoaderManipulator man);
		 
      protected:
         Clause mClause;
         std::shared_ptr<Factor> mFactor;

      };

      /// Insert Value into first row (Instance=0) of this Factor and returns FactorLoader 
      /// to facilitate consequitive load operations in chained fashion
      /// @param v ValueType Value to be assigned to row at Instance=0
      /// @retrun FactorLoader constructed with this Factor preset to Insert at Instance=1
	  FactorLoader operator << (ValueType v);

    protected:

        void Init();

        std::bitset<MAX_SET_SIZE> mBsPresent;
        std::map<VarId, int> mVarToIndex;      // variable to offset in varset
        VarSet mSet;
        unsigned int mFactorSize;
        std::vector<ValueType> mValues;
        std::vector<bool> mValuePresent;
        std::vector<InstanceId> mExtendedClauseVector;

        VarSet mClauseHead;  // refactor into mVarsetHead
        VarSet mExtendedVarSet;
        FactorExtender *mpExtender;

		VarType mFactorType;
   };

   /// fin manipulator is used to complete load Factor values by calculating complimentary
   /// values to insure 1.0 sum of probabilities for same Tail Clause instances
   /// @ingroup API
   inline void fin(std::shared_ptr<Factor> f)
	   f->CompleteProbabilities();
   }


   /// @ingroup API
   /// Implements  Decision Node on Bayesian Decision network
   /// @ingroup API
   class DecisionFunction : public Factor
   {
      public:
      DecisionFunction(const VarSet &vset);
	   DecisionFunction(std::shared_ptr<Factor> f, VarId decisionNode);


      bool GetDecision(InstanceId);

      // from UIElem
      virtual std::string GetJson(VarDb &db) const override;
      virtual std::string GetType() const override;

      protected:
      VarId mDecisionNode;

   };

   /// Contains results of Decision Calculation run
   /// @ingroup API
	class DecisionBuilderHelper : public UIElem
	{
	public:
      void AddDecisionFunction(std::shared_ptr<DecisionFunction> df)
      {
         mDecisions.push_back(df);
      }

      // from UIElem
      virtual std::string GetJson(VarDb &db) const override;
      virtual std::string GetType() const override ;

      /// Get Avaialble decisions given input sample
      /// @param sample Input sample that applied to the model 
      /// @return ClausValue containing all decisions resolved by the sample
	  ClauseValue GetDecisions(const Clause &sample);

     /// Get size of resolved Decision list
     std::size_t GetSize() { return mDecisions.size(); }

   protected:
   
      typedef std::list<std::shared_ptr<DecisionFunction> > ListDecisions;
      ListDecisions mDecisions;

	};

   /// This class represents entire Bayesian Network model
   /// It contains all Factors defined on this domain
   /// @ingroup API
   class FactorSet : public UIElem
   {
   public:

      enum DebugLevel
      {
         DebugLevel_None = 0,
         DebugLevel_Errors = 1,
         DebugLevel_Minor = 2,
         DebugLevel_Details = 3

      };

      /// Construct empty FactorSet
      /// @param db VarDb database of domain variables 
      FactorSet(VarDb &db);

      /// Add Factor for FactorSet
      /// @param f Factor to add  
      void AddFactor(std::shared_ptr<Factor> f);

      /// Merge all Factors in this FactorSet into single Factor
      /// @return merged Factor 
      std::shared_ptr<Factor> Merge();

      /// Get VarSet of all domain variables currently in FactorSet
      /// @return VarSet of all variables in this VarSet
      std::shared_ptr<VarSet> GetVarSet() const;

      /// Get VarSet of all variables of particulat VarType that are ancestors of the node 
      /// @param vid Variable which ancestors will be searched
      /// @param vartype VarType of variables that will be looked up
      /// db database of Domain variables
      std::shared_ptr<VarSet> GetTypedVarSet(VarId vid, VarType vartype, VarDb &db);

      /// Get all ancestors of the node
      /// @param vid VarId of child node 
      /// @return uncestors of child node
      std::shared_ptr<VarSet> GetAncestors(VarId vid);

      /// Get all ancestors of particular type
      /// @param vid VarId of child node 
      /// @param vartype VarType type of ancestors to look 
      /// @return uncestors of child node
      std::shared_ptr<VarSet> GetAncestors(VarId vid, VarType vartype);

      /// Get all leaf nodes of the Beysian graph represented by this FactorSet
      /// @return VarSet representing all VarIds of leaf nodes
      std::shared_ptr<VarSet> GetLeafNodes() const;

      /// Run eliminate Variable algorithm on this FactorSet
      /// @param vs VarSet of variables that will be eleminated
      void EliminateVar(const VarSet &vs);

      /// Run Maximize Variable algorithm on this FactorSet
      /// @param vs VarSet of variables that will be maximized
      void MaximizeVar(const VarSet &vs);

      /// Remove all Factors that contain given Variables in its Head VarSet
      /// @param vs VarSet of variables present in Factor's Head to be erased
      void RemoveVars(const VarSet &vs);

      /// Remove Leaf Factors that are pointed by Variables in passed VarSet
      /// @param vs VarSet of variables that will be be searched in the Head of leaf Factors. The matdhed factors will be removed
      void PruneVars(const VarSet &vs);

      /// Remove Edes (rows in the factors) that contradict to the passed Clause
      /// @param c Clause that will be applied on Factors and contradicting rows will be removed
      void PruneEdges(const Clause &c);

      /// reduce Factros in this FactorSet by applying values assigned to variables in the passed clause and then removing 
      /// this Variables from the factors
      /// @param c Clause to be applied
      void ApplyClause(const Clause &c);

      /// Solve this FactorSet to build DecisionBuildHelper which can be used to query the decisions based on Samples of Data
      /// @return DecisionBuilderHelper containing solution for this FactorSet
      std::shared_ptr<DecisionBuilderHelper> BuildDecision();

      /// Check if this FactorSet is empty
      /// @return true if FactorSet contains no Factors
      bool IsEmpty()
      {
         return mFactors.empty();
      }

      /// ListFactors is a list of Factors
      typedef std::list<std::shared_ptr<Factor> > ListFactors;

      /// Get list of all factors int this FactorSet
      /// @return ListFactors list of all factors in this FactorSet
      const ListFactors &GetFactors() { return mFactors; }


      // from UIElem
      /// Get string Json representations of this FactorSet
      /// @param db VarDb database containing domain Variables
      /// @return string containing Json representation of this FactorSet
      virtual std::string GetJson(VarDb &db) const override;

      /// Returns "FactorSet"
      virtual std::string GetType() const override;

      int GetDebugLevel() { return mDebugLevel; };
      void SetDebugLevel(int n) { mDebugLevel = n; }



   protected:
      ListFactors mFactors;
      VarDb &mDb;
      int mDebugLevel;

   };


   class FactorMergeHelper
   {
   public:
      FactorMergeHelper(const VarSet &pV1,
         const VarSet &pV2);

      int  MapOffsRTo1(int n);
      int  MapOffsRTo2(int n);


      VarSet mV1;
      VarSet mV2;
      VarSet mVr;   // merged varset

      std::map<int, int> mMap_Offs1_Offs2;
      std::map<int, int> mMap_Offs1_OffsR;
      std::map<int, int> mMap_Offs2_OffsR;

      std::map<int, int> mMap_OffsR_Offs1;
      std::map<int, int> mMap_OffsR_Offs2;

   };

   // extern VarDb gVarDb;

   class InteractionGraph
   {
   public:
      InteractionGraph(FactorSet *pFs);

      VarSet GetElimOrder();



   protected:

      bool FindEdge(VarId id1, VarId id2);

      VarSet mVarSet;
      using EdgesMap = std::multimap<VarId, VarId>;
      EdgesMap  mEdges;

   };


}

/// @}

#endif

