/**
 * This file is part of node-kdtree, a node.js Addon for working with kd-trees.
 * Copyright (C) 2011 Justin Ethier <justinethier@github>
 *
 * Please use github to submit patches and bug reports:
 * https://github.com/justinethier/node-kdtree
 */

#include <v8.h>
#include <node.h>
#include <node_version.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <kdtree.h>

using namespace v8;
using namespace node;

/**
 * Free memory allocated to the 'data' portion of a node...
 */
void freeNodeData(void *data){
  if (data != NULL) {
    // Release this persistent handle's storage cell
    Persistent<Value> hData = Persistent<Value>((Value *)data);
    hData.Dispose();
  }
}

/**
 * The KDTree add-on
 */
class KDTree : public ObjectWrap {
  public:
    static void
    Initialize (v8::Handle<v8::Object> target){
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(String::NewSymbol("KDTree"));

        NODE_SET_PROTOTYPE_METHOD(t, "dimensions", Dimensions);
        NODE_SET_PROTOTYPE_METHOD(t, "insert", Insert);
        NODE_SET_PROTOTYPE_METHOD(t, "nearest", Nearest);
        NODE_SET_PROTOTYPE_METHOD(t, "nearestPoint", NearestPoint);
        NODE_SET_PROTOTYPE_METHOD(t, "nearestValue", NearestValue);
        NODE_SET_PROTOTYPE_METHOD(t, "nearestRange", NearestRange);

        target->Set(String::NewSymbol("KDTree"), t->GetFunction());
    }

    /**
     * Getter for tree's dimensions.
     *
     * @return Dimensions of the tree's points.
     */
    int Dimensions(){
      return dim_;
    }

    /**
     * Insert a set of points into the tree.
     *
     * An optional data argument is present as well; 
     * if len == dim_, then the data param will be ignored.
     *
     * @param pos   An array of points
     * @param len   Number of points in the array
     * @param data  Optional data argument
     *
     * @return true if the point was inserted successfully, false otherwise
     */ 
    bool Insert(const double *pos, int len, Handle<Value> data){
      if (len != dim_ && len != dim_ + 1){
        ThrowException(Exception::Error(String::New("Insert(): Wrong number of parameters."))); 
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      if (len == dim_)
        return (kd_insert(kd_, pos, NULL) == 0);
      return (kd_insert(kd_, pos, *data) == 0);
    }

    /**
     * Find the point nearest to the given point.
     *
     * @param pos   An array of points
     * @param len   Number of points in the array
     *
     * @return An array containing the nearest point, or an empty array if no point is found.
     *         If a data element was provided for the nearest point, it will be the last
     *         member of the returned array.
     */ 
    Handle<Value>
    Nearest(const double *pos, int len){
      HandleScope scope;
      int rpos;
      void *pdata;

      if (len != dim_){
        ThrowException(Exception::Error(String::New("Nearest(): Wrong number of parameters."))); 
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      kdres *results = kd_nearest(kd_, pos);
      Local<Array> rv = Array::New(dim_ + 1);
      
      if (results != NULL) {
        double *respos = (double *)(malloc(sizeof(double) * dim_));
        pdata = (void *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rv->Set(rpos, Number::New(respos[rpos])); 
        }

        // Append data element, if present
        if (pdata != NULL) {
          Persistent<Value> hdata = Persistent<Value>((Value *)pdata);
          rv->Set(dim_, hdata); 
        }

        free(respos);
        kd_res_free(results);
      }
      return scope.Close(rv);
    }

    /**
     * Find the points nearest to the given point, within a given range.
     *
     * @param pos   An array of points
     * @param len   Number of points in the array
     * @param range Range in which to search for points
     *
     * @return An array containing the nearest points, or an empty array if no point is found.
     *         If a data element was provided for any point, it will be the last
     *         member of that point's returned array.
     */ 
    Handle<Value> 
    NearestRange(const double *pos, int len, double range){
      HandleScope scope;
      int rpos, i = 0;
      void *pdata;
      kdres *results = NULL; 
      Local<Array> rv = Array::New();

      if (len != dim_){
        ThrowException(Exception::Error(String::New("Nearest(): Wrong number of parameters."))); 
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      results = kd_nearest_range(kd_, pos, range);
      while (!kd_res_end( results )){
        Local<Array> rvItem = Array::New(dim_ + 1);
        double *respos = (double *)(malloc(sizeof(double) * dim_));
        pdata = (void *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rvItem->Set(rpos, Number::New(respos[rpos])); 
        }

        // Append data element, if present
        if (pdata != NULL) {
          Persistent<Value> hdata = Persistent<Value>((Value *)pdata);
          rvItem->Set(dim_, hdata); 
        }

        rv->Set(i++, rvItem);
        free(respos);

        // Move to next result entry
        kd_res_next( results );
      }

      kd_res_free(results);
      return scope.Close(rv);
    }

  protected:

    static Handle<Value>
    Dimensions (const Arguments& args){
        KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
        HandleScope scope;

        return scope.Close(Number::New(kd->Dimensions()));
    }

    /**
     * Wrapper for Insert()
     */
    static Handle<Value>
    Insert(const Arguments& args){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      HandleScope scope;

      double *pos = (double *)(malloc(sizeof(double) * args.Length()));
      for (int i = 0; i < args.Length(); i++){
        pos[i] = args[i]->NumberValue();
      }

      Handle<Value> result = Boolean::New( kd->Insert(pos, args.Length(),
          Persistent<Value>::New(args[ args.Length() - 1])));
      free(pos);
      return scope.Close(result);
    }

    /**
     * Wrapper for Nearest()
     */ 
    static Handle<Value>
    Nearest(const Arguments& args){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      HandleScope scope;

      double *pos = (double *)(malloc(sizeof(double) * args.Length()));
      for (int i = 0; i < args.Length(); i++){
        pos[i] = args[i]->NumberValue();
      }

      Handle<Value> result = kd->Nearest(pos, args.Length()); 
        
      free(pos);
      return scope.Close(result);
    }

    /**
     * A shortcut method for Nearest, that returns an array containing only point data.
     * If a value is present, it will NOT be returned in the result array.
     */
    static Handle<Value>
    NearestPoint(const Arguments& args){
      HandleScope scope;
      Handle<Array> nearest = ((KDTree::Nearest(args))).As<Array>();
      int dim = KDTree::Dimensions(args).As<Number>()->Value();

      Local<Array> result = Array::New(dim); 
      if (nearest->Length() > 0 &&    // Data present
          (int)nearest->Length() >= dim) { // Points present
         for (int i = 0; i < dim; i++) {
           result->Set(i, nearest->Get(i));
         }
      }

      return scope.Close(result);
    }

    /**
     * A shortcut method for Nearest, that returns the point's value as a scalar.
     * If a value is not present or no point was found, this method returns null.
     *
     * For example:
     *
     *  > tree.insert(1, 1, 1, "My Value");
     *  > tree.nearestValue(1, 1, 1);
     *  "My Value"
     *
     */
    static Handle<Value>
    NearestValue(const Arguments& args){
      HandleScope scope;
      Handle<Array> nearest = ((KDTree::Nearest(args))).As<Array>();
      int dim = KDTree::Dimensions(args).As<Number>()->Value();

      if (nearest->Length() > 0 &&         // Data present
          nearest->Length() == (dim + 1)){ // Value present
        return scope.Close( nearest->Get(nearest->Length() - 1) );
      } else {
        return scope.Close( Null() );
      }
    }

    static Handle<Value>
    NearestRange(const Arguments& args){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      HandleScope scope;
      Handle<Value> result; 

      if (args.Length() == 0) {
        ThrowException(Exception::Error(String::New("NearestRange(): No parameters were provided."))); 
      }
      else {
        double *pos = (double *)(malloc(sizeof(double) * args.Length() - 1));
        for (int i = 0; i < args.Length() - 1; i++){
          pos[i] = args[i]->NumberValue();
        }

        result = kd->NearestRange(pos, args.Length() - 1, args[args.Length() - 1]->NumberValue()); 
        free(pos);
      }

      return scope.Close(result);
    }

    /**
     * "External" constructor called by the Addon framework
     */
    static Handle<Value>
    New (const Arguments& args){
        HandleScope scope;

        int dimension = 3; // Default
        if (args.Length() > 0){
          dimension = args[0]->Int32Value();
        }

        KDTree *kd = new KDTree(dimension);
        kd->Wrap(args.This());

        return scope.Close(args.This());
    }

    /**
     * Constructor
     *
     * @param dim   Dimensions of each point in the tree
     */
    KDTree (int dim) : ObjectWrap (){
        kd_ = kd_create(dim);
        dim_ = dim;
        kd_data_destructor(kd_, freeNodeData);
    }

    /**
     * Destructor
     */
    ~KDTree(){
        if (kd_ != NULL){
            kd_free(kd_);
        }
    }

  private:
    /**
     * Pointer to the tree itself
     */
    kdtree* kd_;

    /**
     * Dimension of each point in the tree
     */
    int dim_;
};

/**
 * Entry point required by node.js framework
 */
#if NODE_MAJOR_VERSION == 0 && NODE_MINOR_VERSION < 10
extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    KDTree::Initialize(target);
}
#else
NODE_MODULE(kdtree,KDTree::Initialize)
#endif
