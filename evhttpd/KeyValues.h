/* 
 * File:   KeyValues.h
 * Author: try
 *
 * Created on 2011年6月4日, 下午5:51
 */

#ifndef _KEYVALUES_H
#define	_KEYVALUES_H

#include <vector>
#include <ext/hash_map>
#include <string>
#include <iostream>
#include "resources.h"

/**
 * 表示一键可多值情况的键值对， 模板变量T_KeyComp为键比较器, 当前已有：
 * HashMapStrCompare和HashMapStrIgnoreCaseCompare两种实现，前者区分大小写，后
 * 者不区分大小写。
 * 
 * 注意：此容器中只存键和值的引用地址，不存实际数据内容
 */
template<typename T_KeyComp = HashMapStrCompare>
class KeyValues {
    
public:
    //存放值的结构，实现为值链表
    typedef struct Value{
        const char* text; //具体值
        Value* next; //下一个值
    public:
        Value(const char* text):text(text), next(NULL){}
    }Value;    
    
    typedef __gnu_cxx::hash_map<const char*, Value*, __gnu_cxx::hash<const char*>, T_KeyComp> Data;
    Data data;
    
public:
    KeyValues(){
    }
    virtual ~KeyValues(){
        clear();
    }

    /** 清除数据 */
    void clear(){
        Value* hv;
        Value* tv;
        typename Data::iterator it = data.begin();
        for(; it != data.end(); it++){
            hv = it->second;
            while(hv){
                tv = hv;
                hv = hv->next;
                delete tv;
            }
        }
        data.clear();
    }

    /** 删除一个数据, 如果指定Key没有找到没,返回false */
    bool remove(const char* name){
        if(!name || name[0]==0)return true;
        typename Data::iterator it = data.find(name);
        if(it != data.end()){
            Value* tv;
            Value* hv = it->second;
            while(hv){
                tv = hv;
                hv = hv->next;
                delete tv;
            }
            data.erase(it);
            return true;
        }
        return false;
    }

    /** 新增或追加消息头 */
    void add(const char* name, const char* value){
        if(!name || name[0]==0)return;
        typename Data::iterator it = data.find(name);
        if(it != data.end()){
            Value* hv = it->second;
            while(hv->next){
                hv = hv->next;
            }
            hv->next = new Value(value);
        }else{
            data.insert(typename Data::value_type(name, new Value(value)));
        }
    }


    void set(const char* name, const char* value){
        if(!name || name[0]==0)return;
        typename Data::iterator it = data.find(name);
        if(it != data.end()){
            Value* hv = it->second->next;
            Value* tv;
            while(hv){
                tv = hv;
                hv = hv->next;
                delete tv;
            }
            it->second->text = value;
            it->second->next = NULL;
        }else{
            data.insert(typename Data::value_type(name, new Value(value)));
        }
    }

    /** 返回头信息 */
    const char* get(const char* name){
        if(!name || name[0]==0)return NULL;
        typename Data::iterator it = data.find(name);
        if(it != data.end()){
            return it->second->text;
        }
        return NULL;
    }

    /** 在vector中返回头信息，一键多值 */
    std::vector<const char*>& gets(const char* name, std::vector<const char*>& rsv){
        if(!name || name[0]==0)return rsv;
        typename Data::iterator hit = data.find(name);
        if(hit != data.end()){
            Value* hv = hit->second;//&(hit->second->value);
            while(hv){
                rsv.push_back(hv->text);
                //下一个值
                hv = hv->next;
            }
        }
        return rsv;
    }

    /** 返回值结构 */
    const Value* gets(const char* name){
        if(!name || name[0]==0)return NULL;
        typename Data::iterator it = data.find(name);
        if(it != data.end()){
            return it->second;
        }
        return NULL;
    }

    /** 返回所有消息头名称 */
    std::vector<const char*>& getNames(std::vector<const char*>& rsv){
        if(data.size() > 0){
            typename Data::iterator hit = data.begin();
            for(; hit != data.end(); hit++){
                rsv.push_back(hit->first);
            }
        }
        return rsv;
    }

    /** 值返回值为以key为单位的数量，与一键多值只计数1 */
    unsigned int size(){
        return data.size();
    }


};

#endif	/* _KEYVALUES_H */

