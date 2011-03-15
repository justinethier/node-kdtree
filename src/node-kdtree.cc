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

/**
 * Free memory allocated to the 'data' portion of a node...
 */
void freeNodeData(void *data){
  if (data != NULL) {
    // Release this persistent handle's storage cell
    Persistent<Value> hData = Persistent<Value>::Persistent((Value *)data);
    hData.Dispose();
  }
}

class KDTree : public ObjectWrap {
  public:
    static void
    Initialize (v8::Handle<v8::Object> target){
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(String::NewSymbol("KDTree"));

        NODE_SET_PROTOTYPE_METHOD(t, "insert", Insert);
        NODE_SET_PROTOTYPE_METHOD(t, "nearest", Nearest);
//        NODE_SET_PROTOTYPE_METHOD(t, "nearestRange", NearestRange); // kd_nearest_range

        target->Set(String::NewSymbol("KDTree"), t->GetFunction());
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
      if (len == dim_)
        return (kd_insert(kd_, pos, NULL) == 0);
      return (kd_insert(kd_, pos, *data) == 0);
    }

    /**
     * Find the point nearest to the given point.
     *
     * @return An array containing the nearest point, or an empty array if no point is found.
     *         If a data element was provided for the nearest point, it will be the last
     *         member of the returned array.
     */ 
    Handle<Value>
    Nearest(const double *pos){
      int rpos;
      void *pdata;
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
          Persistent<Value> hdata = Persistent<Value>::Persistent((Value *)pdata);
          rv->Set(dim_, hdata); 
        }

        free(respos);
        kd_res_free(results);
      }
      return rv;
    }

    /*int Test()
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
    }*/

  protected:

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
            // Test code, just stuff number as data for now...
            // Eventually will want API to allow an optional data arg.
            // If not specified, will default to something (null?)
      free(pos);
      return result;
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

      // TODO: may want to pass Length down, so class can assert == dim_ 
      Handle<Value> result = kd->Nearest(pos); 
        
      free(pos);
      return result;
    }

// TODO:
//    static Handle<Value>
//    NearestRange(const Arguments& args){}

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

        return args.This();
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

    /*static Handle<Value>
    Test (const Arguments& args){
        KDTree *kd = ObjectWrap::Unwrap<KDTree>(args.This());

        HandleScope scope;

        int num = kd->Test();

        return Integer::New(num); // Undefined();
    }*/

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

extern "C" void
init (Handle<Object> target)
{
    HandleScope scope;
    KDTree::Initialize(target);
}

