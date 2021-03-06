//
// Created by 冯鸿杰 on 16/10/31.
//

#include <stdint.h>
#include "LuaObjectDescriptor.h"
#include "LuaContext.h"
#include "LuaDefine.h"
#include "LuaPointer.h"
#include "../../../../../lua-core/src/lua.hpp"

/**
 对象引用回收处理

 @param state Lua状态机

 @return 返回值数量
 */
static int objectReferenceGCHandler(lua_State *state)
{
    using namespace cn::vimfung::luascriptcore;

    //释放对象
    LuaUserdataRef ref = (LuaUserdataRef)lua_touserdata(state, 1);
    LuaObjectDescriptor *descriptor = (LuaObjectDescriptor *)(ref -> value);
    descriptor -> release();

    return 0;
}

cn::vimfung::luascriptcore::LuaObjectDescriptor::LuaObjectDescriptor()
        : _object(NULL)
{

}

cn::vimfung::luascriptcore::LuaObjectDescriptor::LuaObjectDescriptor(const void *object)
{
    setObject(object);
}

cn::vimfung::luascriptcore::LuaObjectDescriptor::~LuaObjectDescriptor()
{
    _object = NULL;
}

void cn::vimfung::luascriptcore::LuaObjectDescriptor::setObject(const void *object)
{
    _object = (void *)object;
}

const void* cn::vimfung::luascriptcore::LuaObjectDescriptor::getObject()
{
    return _object;
}

void cn::vimfung::luascriptcore::LuaObjectDescriptor::setReferenceId(const std::string &refId)
{
    _refId = refId;
}

std::string cn::vimfung::luascriptcore::LuaObjectDescriptor::getReferenceId()
{
    return _refId;
}

void cn::vimfung::luascriptcore::LuaObjectDescriptor::push(cn::vimfung::luascriptcore::LuaContext *context)
{
    lua_State *state = context -> getLuaState();

    //创建userdata
    LuaUserdataRef ref = (LuaUserdataRef)lua_newuserdata(state, sizeof(LuaUserdataRef));
    ref -> value = this;
    this -> retain();

    //设置userdata的元表
    luaL_getmetatable(state, "_ObjectReference_");
    if (lua_isnil(state, -1))
    {
        lua_pop(state, 1);

        //尚未注册_ObjectReference,开始注册对象
        luaL_newmetatable(state, "_ObjectReference_");

        lua_pushcfunction(state, objectReferenceGCHandler);
        lua_setfield(state, -2, "__gc");
    }
    lua_setmetatable(state, -2);
}