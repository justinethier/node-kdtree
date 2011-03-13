/**
 * This file is part of node-kdtree, a node.js Addon for working with kd-trees.
 * Copyright (C) 2011 Justin Ethier <justinethier@github>
 *
 * Please use github to submit patches and bug reports:
 * https://github.com/justinethier/node-kdtree
 */

// TODO: publish via npm. see: https://github.com/isaacs/npm/blob/master/doc/developers.md#readme
//
#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <kdtree.h>

using namespace v8;
using namespace node;

class KDTree : public ObjectWrap {
  public:
    static void
    Initialize (v8::Handle<v8::Object> target){
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(String::NewSymbol("KDTree"));

        NODE_SET_PROTOTYPE_METHOD(t, "test", Test); // TODO: this will go away...
        NODE_SET_PROTOTYPE_METHOD(t, "insert", Insert);
        NODE_SET_PROTOTYPE_METHOD(t, "nearest", Nearest);
//        NODE_SET_PROTOTYPE_METHOD(t, "nearestRange", NearestRange); // kd_nearest_range

        target->Set(String::NewSymbol("KDTree"), t->GetFunction());
    }

    // TODO: allow insertion of (string?) data
    bool Insert(const double *pos){
      return (kd_insert(kd_, pos, 0) == 0);
    }

    // Allow retrieval of (string?) data
    Handle<Value>
    Nearest(const double *pos){
      int rpos;
      char *pdata;
      kdres *results = kd_nearest(kd_, pos);
      Local<Array> rv = Array::New(dim_ + 1);

      if (results == NULL){}
      else{
        double *respos = (double *)(malloc(sizeof(double) * dim_));
        pdata = (char *)kd_res_item(results, respos); 

        for(rpos = 0; rpos < dim_; rpos++){
          rv->Set(rpos, Number::New(respos[rpos])); 
        }
       
        free(respos);
        kd_res_free(results);
      }
      return rv;
    }

    int Test()
    {
       int i, vcount = 10;
       kdtree *kd;
       kdres *set;
    //   unsigned int msec, start;
    
       printf("inserting %d random vectors... ", vcount);
       fflush(stdout);
    
       kd = kd_create(3);
    
    //   start = get_msec();
       for(i=0; i<vcount; i++) {
               float x, y, z;
               x = ((float)rand() / RAND_MAX) * 200.0 - 100.0;
               y = ((float)rand() / RAND_MAX) * 200.0 - 100.0;
               z = ((float)rand() / RAND_MAX) * 200.0 - 100.0;
    
               assert(kd_insert3(kd, x, y, z, 0) == 0);
       }
    //   msec = get_msec() - start;
    //   printf("%.3f sec\n", (float)msec / 1000.0);
    
    //   start = get_msec();
       set = kd_nearest_range3(kd, 0, 0, 0, 100);
    //   msec = get_msec() - start;
    //
       i = kd_res_size(set);
       printf("range query returned %d items\n", kd_res_size(set));
       kd_res_free(set);
    
       kd_free(kd);
       return i;
    }

  protected:

    static Handle<Value>
    Insert(const Arguments& args){
        KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
        HandleScope scope;

      double *pos = (double *)(malloc(sizeof(double) * args.Length()));
      for (int i = 0; i < args.Length(); i++){
        pos[i] = args[i]->NumberValue();
      }

      // TODO: may want to pass Length down, so class can assert == dim_ 
      Handle<Value> result = Boolean::New( kd->Insert(pos) );
      free(pos);
      return result;
    }

    static Handle<Value>
    Nearest(const Arguments& args){
      KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());
      HandleScope scope;

      double *pos = (double *)(malloc(sizeof(double) * args.Length()));
      for (int i = 0; i < args.Length(); i++){
        pos[i] = args[i]->NumberValue();
      }

      // TODO: may want to pass Length down, so class can assert == dim_ 
      Handle<Value> result = kd->Nearest(pos);
      free(pos);
      return result;
    }

// TODO:
//    static Handle<Value>
//    NearestRange(const Arguments& args){}

    static Handle<Value>
    New (const Arguments& args){
        HandleScope scope;

// TODO: an example of how to handle arguments to constructor
//Async *async = new Async(args[0]->Int32Value(), args[1]->Int32Value());

        int dimension = 3; // Default
        if (args.Length() > 0){
          dimension = args[3]->Int32Value();
        }

        KDTree *kd = new KDTree(dimension);
        kd->Wrap(args.This());

        return args.This();
    }

    KDTree (int dim) : ObjectWrap (){
        kd_ = kd_create(dim);
        dim_ = dim;
    }

    ~KDTree(){
      // TODO: should probably call this to clean up allocated 'data' elements
      //       see the kdtree docs:
//      void kd_data_destructor(struct kdtree *tree, void (*destr)(void*));
        if (kd_ != NULL){
            kd_free(kd_);
        }
    }

    static Handle<Value>
    Test (const Arguments& args){
        KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());

        HandleScope scope;

        int num = kd->Test();

        return Integer::New(num); // Undefined();
    }

  private:
    kdtree* kd_;
    int dim_;
};

extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    KDTree::Initialize(target);
}

