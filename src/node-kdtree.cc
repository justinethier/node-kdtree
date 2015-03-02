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
#include <nan.h>
#include <kdtree.h>

using namespace v8;
using namespace node;

/**
 * Free memory allocated to the 'data' portion of a node...
 */
void freeNodeData(void *data){
  if (data != NULL) {
    // Release this persistent handle's storage cell
    Persistent<Value>* hData = (Persistent<Value>*)data;
    NanDisposePersistent(*hData);
    delete hData;
  }
}

/**
 * The KDTree add-on
 */
class KDTree : public ObjectWrap {
  public:
    static void
    Initialize (v8::Handle<v8::Object> target){
        NanScope();

        Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(NanNew<String>("KDTree"));

        NODE_SET_PROTOTYPE_METHOD(t, "dimensions", Dimensions);
        NODE_SET_PROTOTYPE_METHOD(t, "insert", Insert);
        NODE_SET_PROTOTYPE_METHOD(t, "nearest", Nearest);
        NODE_SET_PROTOTYPE_METHOD(t, "nearestPoint", NearestPoint);
        NODE_SET_PROTOTYPE_METHOD(t, "nearestValue", NearestValue);
        NODE_SET_PROTOTYPE_METHOD(t, "nearestRange", NearestRange);

        target->Set(NanNew<String>("KDTree"), t->GetFunction());
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
    bool Insert(const double *pos, int len, Persistent<Value>* data){
      if (len != dim_ && len != dim_ + 1){
        NanThrowError("Insert(): Wrong number of parameters.");
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      if (len == dim_)
        return (kd_insert(kd_, pos, NULL) == 0);
      return (kd_insert(kd_, pos, data) == 0);
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
      NanEscapableScope();
      int rpos;
      void *pdata;

      if (len != dim_){
        NanThrowError("Nearest(): Wrong number of parameters.");
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      kdres *results = kd_nearest(kd_, pos);
      Local<Array> rv = NanNew<Array>(dim_ + 1);
      
      if (results != NULL) {
        double *respos = (double *)(malloc(sizeof(double) * dim_));
        pdata = (void *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rv->Set(rpos, NanNew<Number>(respos[rpos])); 
        }

        // Append data element, if present
        if (pdata != NULL) {
          Persistent<Value>* hdata = (Persistent<Value>*)pdata;
          Local<Value> value = NanNew(*hdata);
          rv->Set(dim_, value);
        }

        free(respos);
        kd_res_free(results);
      }
      return NanEscapeScope(rv);
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
      NanEscapableScope();
      int rpos, i = 0;
      void *pdata;
      kdres *results = NULL; 
      Local<Array> rv = NanNew<Array>();

      if (len != dim_){
        NanThrowError("Nearest(): Wrong number of parameters.");
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      results = kd_nearest_range(kd_, pos, range);
      while (!kd_res_end( results )){
        Local<Array> rvItem = NanNew<Array>(dim_ + 1);
        double *respos = (double *)(malloc(sizeof(double) * dim_));
        pdata = (void *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rvItem->Set(rpos, NanNew<Number>(respos[rpos])); 
        }

        // Append data element, if present
        if (pdata != NULL) {
          Persistent<Value>* hdata = (Persistent<Value>*)pdata;
          Local<Value> value = NanNew(*hdata);
          rvItem->Set(dim_, value);
        }

        rv->Set(i++, rvItem);
        free(respos);

        // Move to next result entry
        kd_res_next( results );
      }

      kd_res_free(results);
      return NanEscapeScope(rv);
    }

  protected:

    static Handle<Value> _Dimensions(_NAN_METHOD_ARGS){
        NanEscapableScope();
        KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
        return NanEscapeScope(NanNew<Number>(kd->Dimensions()));
    }

    static NAN_METHOD(Dimensions){
        NanScope();
        NanReturnValue(KDTree::_Dimensions(args));
    }

    /**
     * Wrapper for Insert()
     */
    static NAN_METHOD(Insert){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      NanScope();

      double *pos = (double *)(malloc(sizeof(double) * args.Length()));
      for (int i = 0; i < args.Length(); i++){
        pos[i] = args[i]->NumberValue();
      }

      Persistent<Value>* per = new Persistent<Value>();
      NanAssignPersistent(*per, args[ args.Length() - 1 ]);

      Handle<Value> result = NanNew<Boolean>( kd->Insert(pos, args.Length(), per) );
      free(pos);
      NanReturnValue(result);
    }

    static Handle<Value> _Nearest(_NAN_METHOD_ARGS){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      NanEscapableScope();

      double *pos = (double *)(malloc(sizeof(double) * args.Length()));
      for (int i = 0; i < args.Length(); i++){
        pos[i] = args[i]->NumberValue();
      }

      Handle<Value> result = kd->Nearest(pos, args.Length()); 
        
      free(pos);
      return NanEscapeScope(result);
    }

    /**
     * Wrapper for Nearest()
     */ 
    static NAN_METHOD(Nearest){
      NanScope();
      Handle<Value> result = KDTree::_Nearest(args);
      NanReturnValue(result);
    }

    /**
     * A shortcut method for Nearest, that returns an array containing only point data.
     * If a value is present, it will NOT be returned in the result array.
     */
    static NAN_METHOD(NearestPoint){
      NanScope();
      Handle<Array> nearest = KDTree::_Nearest(args).As<Array>();
      int dim = KDTree::_Dimensions(args).As<Number>()->Value();

      Local<Array> result = NanNew<Array>(dim); 
      if (nearest->Length() > 0 &&    // Data present
          (int)nearest->Length() >= dim) { // Points present
         for (int i = 0; i < dim; i++) {
           result->Set(i, nearest->Get(i));
         }
      }

      NanReturnValue(result);
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
    static NAN_METHOD(NearestValue){
      NanScope();
      Handle<Array> nearest = KDTree::_Nearest(args).As<Array>();
      int dim = KDTree::_Dimensions(args).As<Number>()->Value();

      if (nearest->Length() > 0 &&         // Data present
          (int)nearest->Length() == (dim + 1)){ // Value present
        NanReturnValue( nearest->Get(nearest->Length() - 1) );
      } else {
        NanReturnNull();
      }
    }

    static NAN_METHOD(NearestRange){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      NanScope();
      Handle<Value> result; 

      if (args.Length() == 0) {
        NanThrowError("NearestRange(): No parameters were provided."); 
      }
      else {
        double *pos = (double *)(malloc(sizeof(double) * args.Length() - 1));
        for (int i = 0; i < args.Length() - 1; i++){
          pos[i] = args[i]->NumberValue();
        }

        result = kd->NearestRange(pos, args.Length() - 1, args[args.Length() - 1]->NumberValue()); 
        free(pos);
      }

      NanReturnValue(result);
    }

    /**
     * "External" constructor called by the Addon framework
     */
    static NAN_METHOD(New){
        NanScope();

        int dimension = 3; // Default
        if (args.Length() > 0){
          dimension = args[0]->Int32Value();
        }

        KDTree *kd = new KDTree(dimension);
        kd->Wrap(args.This());

        NanReturnValue(args.This());
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
