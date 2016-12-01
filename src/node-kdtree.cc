/**
 * This file is part of node-kdtree, a node.js Addon for working with kd-trees.
 * Copyright (C) 2011 Justin Ethier <justinethier@github>
 *
 * Please use github to submit patches and bug reports:
 * https://github.com/justinethier/node-kdtree
 */

#include <v8.h>
#include <node.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sstream>
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
    Nan::Persistent<Value>* hData = (Nan::Persistent<Value>*)data;
    hData->Reset();
    delete hData;
  }
}

/**
 * The KDTree add-on
 */
class KDTree : public ObjectWrap {
  public:
    static void
    Initialize (v8::Handle<v8::Object> exports){
        Nan::HandleScope scope;

        Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(Nan::New("KDTree").ToLocalChecked());

        Nan::SetPrototypeMethod(t, "dimensions", Dimensions);
        Nan::SetPrototypeMethod(t, "insert", Insert);
        Nan::SetPrototypeMethod(t, "nearest", Nearest);
        Nan::SetPrototypeMethod(t, "nearestPoint", NearestPoint);
        Nan::SetPrototypeMethod(t, "nearestValue", NearestValue);
        Nan::SetPrototypeMethod(t, "nearestRange", NearestRange);

        exports->Set(Nan::New("KDTree").ToLocalChecked(), t->GetFunction());
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
    bool Insert(const double *pos, int len, Nan::Persistent<Value>* data){
      if (len != dim_ && len != dim_ + 1){
        Nan::ThrowError("Insert(): Wrong number of parameters.");
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
    Local<Value> Nearest(const double *pos, int len){
      Nan::EscapableHandleScope scope;
      int rpos;
      void *pdata;

      if (len != dim_){
        Nan::ThrowError("Nearest(): Wrong number of parameters.");
        // FUTURE: Passed: " + len + " Expected: " + dim_)));
      }

      kdres *results = kd_nearest(kd_, pos);
      Local<Array> rv = Nan::New<Array>(dim_ + 1);
      
      if (results != NULL) {
        double *respos = new double[dim_];
        pdata = (void *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rv->Set(rpos, Nan::New<Number>(respos[rpos])); 
        }

        // Append data element, if present
        if (pdata != NULL) {
          Nan::Persistent<Value>* hdata = (Nan::Persistent<Value>*)pdata;
          Local<Value> value = Nan::New(*hdata);
          rv->Set(dim_, value);
        }

        free(respos);
        kd_res_free(results);
      }
      return scope.Escape(rv);
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
    Local<Value> NearestRange(const double *pos, int len, double range){
      Nan::EscapableHandleScope scope;
      int rpos, i = 0;
      void *pdata;
      kdres *results = NULL; 
      Local<Array> rv = Nan::New<Array>();

      if (len != dim_){
        std::stringstream ss;
        ss << "Nearest(): Wrong number of parameters. Passed: "
           << len << " Expected: " << dim_;
        Nan::ThrowError(Nan::New(ss.str()).ToLocalChecked());
      }

      results = kd_nearest_range(kd_, pos, range);
      while (!kd_res_end( results )){
        Local<Array> rvItem = Nan::New<Array>(dim_ + 1);
        double *respos = new double[dim_];
        pdata = (void *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rvItem->Set(rpos, Nan::New<Number>(respos[rpos])); 
        }

        // Append data element, if present
        if (pdata != NULL) {
          Nan::Persistent<Value>* hdata = (Nan::Persistent<Value>*)pdata;
          Local<Value> value = Nan::New(*hdata);
          rvItem->Set(dim_, value);
        }

        rv->Set(i++, rvItem);
        delete[] respos;

        // Move to next result entry
        kd_res_next( results );
      }

      kd_res_free(results);
      return scope.Escape(rv);
    }

  protected:

    static Local<Value> _Dimensions(Nan::NAN_METHOD_ARGS_TYPE info){
        Nan::EscapableHandleScope scope;
        KDTree *kd = ObjectWrap::Unwrap<KDTree>(info.This());
        return scope.Escape(Nan::New<Number>(kd->dim_));
    }

    static NAN_METHOD(Dimensions){
        info.GetReturnValue().Set(KDTree::_Dimensions(info));
    }

    /**
     * Wrapper for Insert()
     */
    static NAN_METHOD(Insert){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(info.This());

      double *pos = new double[info.Length()];
      for (int i = 0; i < info.Length(); i++){
        pos[i] = info[i]->NumberValue();
      }

      Local<Value> data = info[ info.Length() - 1 ];
      Nan::Persistent<Value>* per = new Nan::Persistent<Value>(data);

      Local<Value> result = Nan::New<Boolean>( kd->Insert(pos, info.Length(), per) );
      delete[] pos;
      info.GetReturnValue().Set(result);
    }

    static Local<Value> _Nearest(Nan::NAN_METHOD_ARGS_TYPE info){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(info.This());
      Nan::EscapableHandleScope scope;

      double *pos = new double[info.Length()];
      for (int i = 0; i < info.Length(); i++){
        pos[i] = info[i]->NumberValue();
      }

      Local<Value> result = kd->Nearest(pos, info.Length()); 
        
      delete[] pos;
      return scope.Escape(result);
    }

    /**
     * Wrapper for Nearest()
     */ 
    static NAN_METHOD(Nearest){
      Nan::HandleScope scope;
      Local<Value> result = KDTree::_Nearest(info);
      info.GetReturnValue().Set(result);
    }

    /**
     * A shortcut method for Nearest, that returns an array containing only point data.
     * If a value is present, it will NOT be returned in the result array.
     */
    static NAN_METHOD(NearestPoint){
      Nan::HandleScope scope;
      Handle<Array> nearest = KDTree::_Nearest(info).As<Array>();
      int dim = KDTree::_Dimensions(info).As<Number>()->Value();

      Local<Array> result = Nan::New<Array>(dim); 
      if (nearest->Length() > 0 &&    // Data present
          (int)nearest->Length() >= dim) { // Points present
         for (int i = 0; i < dim; i++) {
           result->Set(i, nearest->Get(i));
         }
      }

      info.GetReturnValue().Set(result);
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
      Nan::HandleScope scope;
      Handle<Array> nearest = KDTree::_Nearest(info).As<Array>();
      int dim = KDTree::_Dimensions(info).As<Number>()->Value();

      if (nearest->Length() > 0 &&         // Data present
          (int)nearest->Length() == (dim + 1)){ // Value present
        info.GetReturnValue().Set( nearest->Get(nearest->Length() - 1) );
      } else {
        info.GetReturnValue().SetNull();
      }
    }

    static NAN_METHOD(NearestRange){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(info.This());
      Nan::HandleScope scope;
      Local<Value> result; 

      if (info.Length() == 0) {
        Nan::ThrowError("NearestRange(): No parameters were provided."); 
      }
      else {
        double *pos = new double[info.Length() - 1];
        for (int i = 0; i < info.Length() - 1; i++){
          pos[i] = info[i]->NumberValue();
        }

        result = kd->NearestRange(pos, info.Length() - 1, info[info.Length() - 1]->NumberValue()); 
        delete[] pos;
      }

      info.GetReturnValue().Set(result);
    }

    /**
     * "External" constructor called by the Addon framework
     */
    static NAN_METHOD(New){
        int dimension = 3; // Default
        if (info.Length() > 0){
          dimension = info[0]->Int32Value();
        }

        KDTree *kd = new KDTree(dimension);
        kd->Wrap(info.This());

        info.GetReturnValue().Set(info.This());
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
NODE_MODULE(kdtree, KDTree::Initialize)

