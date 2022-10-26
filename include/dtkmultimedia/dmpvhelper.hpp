// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMPV_CLIENT_API_QTHELPER_H_
#define DMPV_CLIENT_API_QTHELPER_H_

#include <mpv/client.h>	

#if !MPV_ENABLE_DEPRECATED	
#error "This helper is deprecated. Copy it into your project instead."	
#else	


#include <cstring>	

#include <QVariant>	
#include <QString>	
#include <QList>	
#include <QHash>	
#include <QSharedPointer>	
#include <QMetaType>	
#include "dtkmultimedia.h"
DMULTIMEDIA_BEGIN_NAMESPACE

class Handle	
{	
    struct container {	
        explicit container(mpv_handle *h) : mpv(h) {}
        ~container() { mpv_terminate_destroy(mpv); }	
        mpv_handle *mpv;	
    };	
    QSharedPointer<container> sptr;	
public:	
    static Handle FromRawHandle(mpv_handle *handle) {	
        Handle h;	
        h.sptr = QSharedPointer<container>(new container(handle));	
        return h;	
    }	

    operator mpv_handle*() const { return sptr ? (*sptr).mpv : 0; }	
};	

static inline QVariant node_to_variant(const mpv_node *node)	
{	
    switch (node->format) {	
    case MPV_FORMAT_STRING:	
        return QVariant(QString::fromUtf8(node->u.string));	
    case MPV_FORMAT_FLAG:	
        return QVariant(static_cast<bool>(node->u.flag));	
    case MPV_FORMAT_INT64:	
        return QVariant(static_cast<qlonglong>(node->u.int64));	
    case MPV_FORMAT_DOUBLE:	
        return QVariant(node->u.double_);	
    case MPV_FORMAT_NODE_ARRAY: {	
        mpv_node_list *list = node->u.list;	
        QVariantList qlist;	
        for (int n = 0; n < list->num; n++)	
            qlist.append(node_to_variant(&list->values[n]));	
        return QVariant(qlist);	
    }	
    case MPV_FORMAT_NODE_MAP: {	
        mpv_node_list *list = node->u.list;	
        QVariantMap qmap;	
        for (int n = 0; n < list->num; n++) {	
            qmap.insert(QString::fromUtf8(list->keys[n]),	
                        node_to_variant(&list->values[n]));	
        }	
        return QVariant(qmap);	
    }	
    default: 	
        return QVariant();	
    }	
}	

struct node_builder {	
    explicit node_builder(const QVariant& v) {
        set(&node_, v);	
    }	
    ~node_builder() {	
        free_node(&node_);	
    }	
    mpv_node *node() { return &node_; }	
private:	
    Q_DISABLE_COPY(node_builder)	
    mpv_node node_;	
    mpv_node_list *create_list(mpv_node *dst, bool is_map, int num) {	
        dst->format = is_map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;	
        mpv_node_list *list = new mpv_node_list();	
        dst->u.list = list;	
        if (!list)	
            goto err;	
        list->values = new mpv_node[num]();	
        if (!list->values)	
            goto err;	
        if (is_map) {	
            list->keys = new char*[num]();	
            if (!list->keys)	
                goto err;	
        }	
        return list;	
    err:	
        free_node(dst);	
        return NULL;	
    }	
    char *dup_qstring(const QString &s) {	
        QByteArray b = s.toUtf8();	
        char *r = new char[b.size() + 1];	
        if (r)	
            std::memcpy(r, b.data(), b.size() + 1);	
        return r;	
    }	
    bool test_type(const QVariant &v, QMetaType::Type t) {		
        return static_cast<int>(v.type()) == static_cast<int>(t);	
    }	
    void set(mpv_node *dst, const QVariant &src) {	
        if (test_type(src, QMetaType::QString)) {	
            dst->format = MPV_FORMAT_STRING;	
            dst->u.string = dup_qstring(src.toString());	
            if (!dst->u.string)	
                goto fail;	
        } else if (test_type(src, QMetaType::Bool)) {	
            dst->format = MPV_FORMAT_FLAG;	
            dst->u.flag = src.toBool() ? 1 : 0;	
        } else if (test_type(src, QMetaType::Int) ||	
                   test_type(src, QMetaType::LongLong) ||	
                   test_type(src, QMetaType::UInt) ||	
                   test_type(src, QMetaType::ULongLong))	
        {	
            dst->format = MPV_FORMAT_INT64;	
            dst->u.int64 = src.toLongLong();	
        } else if (test_type(src, QMetaType::Double)) {	
            dst->format = MPV_FORMAT_DOUBLE;	
            dst->u.double_ = src.toDouble();	
        } else if (src.canConvert<QVariantList>()) {	
            QVariantList qlist = src.toList();	
            mpv_node_list *list = create_list(dst, false, qlist.size());	
            if (!list)	
                goto fail;	
            list->num = qlist.size();	
            for (int n = 0; n < qlist.size(); n++)	
                set(&list->values[n], qlist[n]);	
        } else if (src.canConvert<QVariantMap>()) {	
            QVariantMap qmap = src.toMap();	
            mpv_node_list *list = create_list(dst, true, qmap.size());	
            if (!list)	
                goto fail;	
            list->num = qmap.size();	
            for (int n = 0; n < qmap.size(); n++) {	
                list->keys[n] = dup_qstring(qmap.keys()[n]);	
                if (!list->keys[n]) {	
                    free_node(dst);	
                    goto fail;	
                }	
                set(&list->values[n], qmap.values()[n]);	
            }	
        } else {	
            goto fail;	
        }	
        return;	
    fail:	
        dst->format = MPV_FORMAT_NONE;	
    }	
    void free_node(mpv_node *dst) {	
        switch (dst->format) {	
        case MPV_FORMAT_STRING:	
            delete[] dst->u.string;	
            break;	
        case MPV_FORMAT_NODE_ARRAY:	
        case MPV_FORMAT_NODE_MAP: {	
            mpv_node_list *list = dst->u.list;	
            if (list) {	
                for (int n = 0; n < list->num; n++) {	
                    if (list->keys)	
                        delete[] list->keys[n];	
                    if (list->values)	
                        free_node(&list->values[n]);	
                }	
                delete[] list->keys;	
                delete[] list->values;	
            }	
            delete list;	
            break;	
        }	
        default: ;	
        }	
        dst->format = MPV_FORMAT_NONE;	
    }	
};	

struct node_autofree {	
    mpv_node *ptr;	
    explicit node_autofree(mpv_node *a_ptr) : ptr(a_ptr) {}
    ~node_autofree() { mpv_free_node_contents(ptr); }	
};	

#if MPV_ENABLE_DEPRECATED	

static inline QVariant get_property_variant(mpv_handle *ctx, const QString &name)	
{	
    mpv_node node;	
    if (mpv_get_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, &node) < 0)	
        return QVariant();	
    node_autofree f(&node);	
    return node_to_variant(&node);	
}	

static inline int set_property_variant(mpv_handle *ctx, const QString &name,	
                                       const QVariant &v)	
{	
    node_builder node(v);	
    return mpv_set_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());	
}	

static inline int set_option_variant(mpv_handle *ctx, const QString &name,	
                                     const QVariant &v)	
{	
    node_builder node(v);	
    return mpv_set_option(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());	
}	

static inline QVariant command_variant(mpv_handle *ctx, const QVariant &args)	
{	
    node_builder node(args);	
    mpv_node res;	
    if (mpv_command_node(ctx, node.node(), &res) < 0)	
        return QVariant();	
    node_autofree f(&res);	
    return node_to_variant(&res);	
}	

#endif	

struct ErrorReturn	
{	
    int error;	

    ErrorReturn() : error(0) {}	
    explicit ErrorReturn(int err) : error(err) {}	
};	

static inline int get_error(const QVariant &v)	
{	
    if (!v.canConvert<ErrorReturn>())	
        return 0;	
    return v.value<ErrorReturn>().error;	
}	

static inline bool is_error(const QVariant &v)	
{	
    return get_error(v) < 0;	
}	

static inline QVariant get_property(mpv_handle *ctx, const QString &name)	
{	
    mpv_node node;	
    int err = mpv_get_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, &node);	
    if (err < 0)	
        return QVariant::fromValue(ErrorReturn(err));	
    node_autofree f(&node);	
    return node_to_variant(&node);	
}	

static inline int set_property(mpv_handle *ctx, const QString &name,	
                                       const QVariant &v)	
{	
    node_builder node(v);	
    return mpv_set_property(ctx, name.toUtf8().data(), MPV_FORMAT_NODE, node.node());	
}	

static inline QVariant command(mpv_handle *ctx, const QVariant &args)	
{	
    node_builder node(args);	
    mpv_node res;	
    int err = mpv_command_node(ctx, node.node(), &res);	
    if (err < 0)	
        return QVariant::fromValue(ErrorReturn(err));	
    node_autofree f(&res);	
    return node_to_variant(&res);	
}	

DMULTIMEDIA_END_NAMESPACE

Q_DECLARE_METATYPE(DMULTIMEDIA_NAMESPACE::ErrorReturn)

#endif

#endif
