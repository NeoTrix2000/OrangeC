/* Software License Agreement
 * 
 *     Copyright(C) 1994-2019 David Lindauer, (LADSoft)
 * 
 *     This file is part of the Orange C Compiler package.
 * 
 *     The Orange C Compiler package is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     The Orange C Compiler package is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Orange C.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *     contact information:
 *         email: TouchStone222@runbox.com <David Lindauer>
 * 
 */

#include "compiler.h"

extern int codeLabel;

#ifndef CPREPROCESSOR
extern ARCH_DEBUG* chosenDebugger;
extern FILE* listFile;
extern INCLUDES* includes;
extern int structLevel;
extern int templateNestingCount;
#endif

NAMESPACEVALUES *globalNameSpace, *localNameSpace;
HASHTABLE* labelSyms;

HASHTABLE* CreateHashTable(int size);
int matchOverloadLevel;

static LIST* usingDirectives;

#ifdef PARSER_ONLY
extern HASHTABLE* kwhash;
extern HASHTABLE* defsyms;
extern HASHTABLE* labelSyms;
extern HASHTABLE* ccHash;
void ccSetSymbol(SYMBOL* s);
#endif

void syminit(void)
{
    globalNameSpace = (NAMESPACEVALUES *)Alloc(sizeof(NAMESPACEVALUES));
    globalNameSpace->syms = CreateHashTable(GLOBALHASHSIZE);
    globalNameSpace->tags = CreateHashTable(GLOBALHASHSIZE);
    localNameSpace = (NAMESPACEVALUES *)Alloc(sizeof(NAMESPACEVALUES));
    usingDirectives = NULL;
    matchOverloadLevel = 0;
}
HASHTABLE* CreateHashTable(int size)
{
    HASHTABLE* rv = (HASHTABLE *)Alloc(sizeof(HASHTABLE));
    rv->table = (HASHREC **)Alloc(sizeof(HASHREC*) * size);
    rv->size = size;
    return rv;
}
#ifndef CPREPROCESSOR
void AllocateLocalContext(BLOCKDATA* block, SYMBOL* sp, int label)
{
    HASHTABLE* tn = CreateHashTable(1);
    STATEMENT* st;
    LIST* l;
    st = stmtNode(NULL, block, st_dbgblock);
    st->label = 1;
    if (block && cparams.prm_debug)
    {
        st = stmtNode(NULL, block, st_label);
        st->label = label;
        tn->blocknum = st->blocknum;
    }
    tn->next = tn->chain = localNameSpace->syms;
    localNameSpace->syms = tn;
    tn = CreateHashTable(1);
    if (block && cparams.prm_debug)
    {
        tn->blocknum = st->blocknum;
    }
    tn->next = tn->chain = localNameSpace->tags;
    localNameSpace->tags = tn;
    if (sp)
        localNameSpace->tags->blockLevel = sp->value.i++;

    l = (LIST *)Alloc(sizeof(LIST));
    l->data = localNameSpace->usingDirectives;
    l->next = usingDirectives;
    usingDirectives = l;
}
#    ifdef PARSER_ONLY
void TagSyms(HASHTABLE* syms)
{
    int i;
    for (i = 0; i < syms->size; i++)
    {
        HASHREC* hr = syms->table[i];
        while (hr)
        {
            SYMBOL* sp = (SYMBOL*)hr->p;
            sp->ccEndLine = includes->line + 1;
            hr = hr->next;
        }
    }
}
#    endif
void FreeLocalContext(BLOCKDATA* block, SYMBOL* sp, int label)
{
    HASHTABLE* locals = localNameSpace->syms;
    HASHTABLE* tags = localNameSpace->tags;
    STATEMENT* st;
    if (block && cparams.prm_debug)
    {
        st = stmtNode(NULL, block, st_label);
        st->label = label;
    }
    checkUnused(localNameSpace->syms);
    if (sp && listFile)
    {
        if (localNameSpace->syms->table[0])
        {
            fprintf(listFile, "******** Local Symbols ********\n");
            list_table(sp->inlineFunc.syms, 0);
            fprintf(listFile, "\n");
        }
        if (localNameSpace->tags->table[0])
        {
            fprintf(listFile, "******** Local Tags ********\n");
            list_table(sp->inlineFunc.tags, 0);
            fprintf(listFile, "\n");
        }
    }
    if (sp)
        sp->value.i--;

    st = stmtNode(NULL, block, st_expr);
    destructBlock(&st->select, localNameSpace->syms->table[0], true);
    localNameSpace->syms = localNameSpace->syms->next;
    localNameSpace->tags = localNameSpace->tags->next;

    localNameSpace->usingDirectives = (LIST *) usingDirectives->data;
    usingDirectives = usingDirectives->next;

#    ifdef PARSER_ONLY
    TagSyms(locals);
    TagSyms(tags);
#    endif
    if (sp)
    {
        locals->next = sp->inlineFunc.syms;
        tags->next = sp->inlineFunc.tags;
        sp->inlineFunc.syms = locals;
        sp->inlineFunc.tags = tags;
    }
    st = stmtNode(NULL, block, st_dbgblock);
    st->label = 0;
}
#endif
/* SYMBOL tab hash function */
static int GetHashValue(const char* string)
{
    unsigned i;
    for (i = 0; *string; string++)
        i = ((i << 7) + (i << 1) + i) ^ *string;
    return i;
}
HASHREC** GetHashLink(HASHTABLE* t, const char* string)
{
    unsigned i;
    if (t->size == 1)
        return &t->table[0];
    for (i = 0; *string; string++)
        i = ((i << 7) + (i << 1) + i) ^ *string;
    return &t->table[i % t->size];
}
/* Add a hash item to the table */
HASHREC* AddName(SYMBOL* item, HASHTABLE* table)
{
    HASHREC** p = GetHashLink(table, item->name);
    HASHREC* newRec;

    if (*p)
    {
        HASHREC *q = *p, *r = *p;
        while (q)
        {
            r = q;
            if (!strcmp(r->p->name, item->name))
                return (r);
            q = q->next;
        }
        newRec = (HASHREC *)Alloc(sizeof(HASHREC));
        r->next = newRec;
        newRec->p = (struct sym *)item;
    }
    else
    {
        newRec = (HASHREC *)Alloc(sizeof(HASHREC));
        *p = newRec;
        newRec->p = (struct sym *)item;
    }
    return (0);
}
HASHREC* AddOverloadName(SYMBOL* item, HASHTABLE* table)
{
    HASHREC** p = GetHashLink(table, item->decoratedName);
    HASHREC* newRec;
#ifdef PARSER_ONLY
    if (!item->parserSet)
    {
        item->parserSet = true;
        ccSetSymbol(item);
    }
#endif

    if (*p)
    {
        HASHREC *q = *p, *r = *p;
        while (q)
        {
            r = q;
            if (!strcmp(((SYMBOL*)r->p)->decoratedName, item->decoratedName))
                return (r);
            q = q->next;
        }
        newRec = (HASHREC *)Alloc(sizeof(HASHREC));
        r->next = newRec;
        newRec->p = (struct sym *)item;
    }
    else
    {
        newRec = (HASHREC *)Alloc(sizeof(HASHREC));
        *p = newRec;
        newRec->p = (struct sym *)item;
    }
    return (0);
}

/*
 * Find something in the hash table
 */
HASHREC** LookupName(const char* name, HASHTABLE* table)
{
    if (table->fast)
    {
        table = table->fast;
    }
    HASHREC** p = GetHashLink(table, name);

    while (*p)
    {
        if (!strcmp((*p)->p->name, name))
        {
            return p;
        }
        p = (HASHREC**)*p;
    }
    return (0);
}
SYMBOL* search(const char* name, HASHTABLE* table)
{
    while (table)
    {
        HASHREC** p = LookupName(name, table);
        if (p)
            return (SYMBOL*)(*p)->p;
        table = table->next;
    }
    return NULL;
}
bool matchOverload(TYPE* tnew, TYPE* told, bool argsOnly)
{
    HASHREC* hnew = basetype(tnew)->syms->table[0];
    HASHREC* hold = basetype(told)->syms->table[0];
    unsigned tableOld[100], tableNew[100];
    int tCount = 0;
    if (!cparams.prm_cplusplus)
        argsOnly = true;
    if (isconst(tnew) != isconst(told))
        return false;
    if (isvolatile(tnew) != isvolatile(told))
        return false;
    if (islrqual(tnew) != islrqual(told))
        return false;
    if (isrrqual(tnew) != isrrqual(told))
        return false;
    matchOverloadLevel++;
    while (hnew && hold)
    {
        SYMBOL* snew = (SYMBOL*)hnew->p;
        SYMBOL* sold = (SYMBOL*)hold->p;
        TYPE *tnew, *told;
        if (sold->thisPtr)
        {
            hold = hold->next;
            if (!hold)
                break;
            sold = (SYMBOL*)hold->p;
        }
        if (snew->thisPtr)
        {
            hnew = hnew->next;
            if (!hnew)
                break;
            snew = (SYMBOL*)hnew->p;
        }
        tnew = basetype(snew->tp);
        told = basetype(sold->tp);
        if (told->type != bt_any || tnew->type != bt_any)  // packed template param
        {
            if ((!comparetypes(told, tnew, true) && !sameTemplatePointedTo(told, tnew)) || told->type != tnew->type)
                break;
            else
            {
                TYPE* tps = sold->tp;
                TYPE* tpn = snew->tp;
                if (isref(tps))
                    tps = basetype(tps)->btp;
                if (isref(tpn))
                    tpn = basetype(tpn)->btp;
                while (ispointer(tpn) && ispointer(tps))
                {
                    if (isconst(tpn) != isconst(tps) || isvolatile(tpn) != isvolatile(tps))
                    {
                        matchOverloadLevel--;
                        return false;
                    }
                    tpn = basetype(tpn)->btp;
                    tps = basetype(tps)->btp;
                }
                if (isconst(tpn) != isconst(tps) || isvolatile(tpn) != isvolatile(tps))
                {
                    matchOverloadLevel--;
                    return false;
                }
                tpn = basetype(tpn);
                tps = basetype(tps);
                if (tpn->type == bt_templateparam)
                {
                    if (tps->type != bt_templateparam)
                        break;
                    if (tpn->templateParam->p->packed != tps->templateParam->p->packed)
                        break;
                    tableOld[tCount] = GetHashValue(tps->templateParam->argsym->name);
                    tableNew[tCount] = GetHashValue(tpn->templateParam->argsym->name);
                    tCount++;
                }
            }
        }
        else
        {
            return false;
        }
        hold = hold->next;
        hnew = hnew->next;
    }
    matchOverloadLevel--;
    if (!hold && !hnew)
    {
        if (tCount)
        {
            int i, j;
            SYMBOL* fnew = basetype(tnew)->sp->parentClass;
            SYMBOL* fold = basetype(told)->sp->parentClass;
            TEMPLATEPARAMLIST *tplNew, *tplOld;
            int iCount = 0;
            unsigned oldIndex[100], newIndex[100];
            tplNew = fnew && fnew->templateParams ? fnew->templateParams->next : NULL;
            tplOld = fold && fold->templateParams ? fold->templateParams->next : NULL;
            while (tplNew && tplOld)
            {
                if (tplOld->argsym && tplNew->argsym)
                {
                    oldIndex[iCount] = GetHashValue(tplOld->argsym->name);
                    newIndex[iCount] = GetHashValue(tplNew->argsym->name);
                    iCount++;
                }
                tplNew = tplNew->next;
                tplOld = tplOld->next;
            }
            for (i = 0; i < tCount; i++)
            {
                int k, l;
                for (k = 0; k < iCount; k++)
                    if (tableOld[i] == oldIndex[k])
                        break;
                for (l = 0; l < iCount; l++)
                    if (tableNew[i] == newIndex[l])
                        break;
                if (k != l)
                {
                    return false;
                }
                for (j = i + 1; j < tCount; j++)
                    if (tableOld[i] == tableOld[j])
                    {
                        if (tableNew[i] != tableNew[j])
                            return false;
                    }
                    else
                    {
                        if (tableNew[i] == tableNew[j])
                            return false;
                    }
            }
        }
        if (basetype(tnew)->sp && basetype(told)->sp)
        {
            if (basetype(tnew)->sp->templateLevel || basetype(told)->sp->templateLevel)
            {
                TYPE* tps = basetype(told)->btp;
                TYPE* tpn = basetype(tnew)->btp;
                if (!templatecomparetypes(tpn, tps, true) && !sameTemplate(tpn, tps))
                {
                    if (isref(tps))
                        tps = basetype(tps)->btp;
                    if (isref(tpn))
                        tpn = basetype(tpn)->btp;
                    while (ispointer(tpn) && ispointer(tps))
                    {
                        if (isconst(tpn) != isconst(tps) || isvolatile(tpn) != isvolatile(tps))
                            return false;
                        tpn = basetype(tpn)->btp;
                        tps = basetype(tps)->btp;
                    }
                    if (isconst(tpn) != isconst(tps) || isvolatile(tpn) != isvolatile(tps))
                        if (basetype(tpn)->type != bt_templateselector)
                            return false;
                    tpn = basetype(tpn);
                    tps = basetype(tps);
                    if (comparetypes(tpn, tps, true) || (tpn->type == bt_templateparam && tps->type == bt_templateparam))
                    {
                        return true;
                    }
                    else if (isarithmetic(tpn) && isarithmetic(tps))
                    {
                        return false;
                    }
                    else if (tpn->type == bt_templateselector)
                    {
                        if (tps->type == bt_templateselector)
                        {
                            if (!templateselectorcompare(tpn->sp->templateSelector, tps->sp->templateSelector))
                            {
                                TEMPLATESELECTOR* ts1 = tpn->sp->templateSelector->next;
                                TEMPLATESELECTOR* ts2 = tps->sp->templateSelector->next;
                                if (ts2->sym->typedefSym)
                                {
                                    ts1 = ts1->next;
                                    if (!strcmp(ts1->name, ts2->sym->typedefSym->name))
                                    {
                                        ts1 = ts1->next;
                                        ts2 = ts2->next;
                                        while (ts1 && ts2)
                                        {
                                            if (strcmp(ts1->name, ts2->name))
                                                return false;
                                            ts1 = ts1->next;
                                            ts2 = ts2->next;
                                        }
                                        if (ts1 || ts2)
                                            return false;
                                    }
                                }
                                else if (!strcmp(tpn->sp->templateSelector->next->name,tps->sp->templateSelector->next->name))
                                {
                                    if (tpn->sp->templateSelector->next->name[0])
                                        return false;
                                }
                            }
                        }
                        else
                        {
                            TEMPLATESELECTOR* tpl = basetype(tpn)->sp->templateSelector->next;
                            SYMBOL* sp = tpl->sym;
                            TEMPLATESELECTOR* find = tpl->next;
                            while (sp && find)
                            {
                                SYMBOL* fsp;
                                if (!isstructured(sp->tp))
                                    break;

                                fsp = search(find->name, basetype(sp->tp)->syms);
                                if (!fsp)
                                {
                                    fsp = classdata(find->name, basetype(sp->tp)->sp, NULL, false, false);
                                    if (fsp == (SYMBOL*)-1)
                                        fsp = NULL;
                                }
                                sp = fsp;
                                find = find->next;
                            }
                            if (find || !sp || (!comparetypes(sp->tp, tps, true) && !sameTemplate(sp->tp, tps)))
                                return false;
                        }
                        return true;
                    }
                    else if (tpn->type == bt_templatedecltype && tps->type == bt_templatedecltype)
                    {
                        return templatecompareexpressions(tpn->templateDeclType, tps->templateDeclType);
                    }
                    return true;
                }
                if (tpn->type == bt_templateselector && tps->type == bt_templateselector)
                {
                    TEMPLATESELECTOR *ts1 = tpn->sp->templateSelector->next, *tss1;
                    TEMPLATESELECTOR *ts2 = tps->sp->templateSelector->next, *tss2;
                    if (ts1->isTemplate != ts2->isTemplate || strcmp(ts1->sym->decoratedName, ts2->sym->decoratedName))
                        return false;
                    tss1 = ts1->next;
                    tss2 = ts2->next;
                    while (tss1 && tss2)
                    {
                        if (strcmp(tss1->name, tss2->name))
                            return false;
                        tss1 = tss1->next;
                        tss2 = tss2->next;
                    }
                    if (tss1 || tss2)
                        return false;
                    if (ts1->isTemplate)
                    {
                        if (!exactMatchOnTemplateParams(ts1->templateParams, ts2->templateParams))
                            return false;
                    }
                    return true;
                    return templateselectorcompare(tpn->sp->templateSelector, tps->sp->templateSelector);  // unreachable
                }
            }
            else if (basetype(tnew)->sp->castoperator)
            {
                TYPE* tps = basetype(told)->btp;
                TYPE* tpn = basetype(tnew)->btp;
                if (!templatecomparetypes(tpn, tps, true) && !sameTemplate(tpn, tps))
                    return false;
            }
            return true;
        }
        else
        {
            return true;
        }
    }
    return false;
}
SYMBOL* searchOverloads(SYMBOL* sp, HASHTABLE* table)
{
    HASHREC* p = table->table[0];
    if (cparams.prm_cplusplus)
    {
        while (p)
        {
            SYMBOL* spp = (SYMBOL*)p->p;
            if (matchOverload(sp->tp, spp->tp, false))
            {
                if (!spp->templateParams)
                    return spp;
                if (sp->templateLevel && !spp->templateLevel && !sp->templateParams->next)
                    return spp;
                if (!!spp->templateParams == !!sp->templateParams)
                {
                    TEMPLATEPARAMLIST* tpl = spp->templateParams->next;
                    TEMPLATEPARAMLIST* tpr = sp->templateParams->next;
                    while (tpl && tpr)
                    {
                        if (tpl->p->type == kw_int && tpl->p->byNonType.tp->type == bt_templateselector)
                            break;
                        if (tpr->p->type == kw_int && tpr->p->byNonType.tp->type == bt_templateselector)
                            break;
                        //    if (tpl->argsym->compilerDeclared || tpr->argsym->compilerDeclared)
                        //        break;
                        tpl = tpl->next;
                        tpr = tpr->next;
                    }
                    if (!tpl && !tpr)
                        return spp;
                }
            }
            p = p->next;
        }
    }
    else
    {
        return (SYMBOL*)p->p;
    }
    return (0);
}
SYMBOL* gsearch(const char* name)
{
    SYMBOL* sp = search(name, localNameSpace->syms);
    if (sp)
        return sp;
    return search(name, globalNameSpace->syms);
}
SYMBOL* tsearch(const char* name)
{
    SYMBOL* sp = search(name, localNameSpace->tags);
    if (sp)
        return sp;
    return search(name, globalNameSpace->tags);
}
void baseinsert(SYMBOL* in, HASHTABLE* table)
{
    if (cparams.prm_extwarning)
        if (in->storage_class == sc_parameter || in->storage_class == sc_auto || in->storage_class == sc_register)
        {
            SYMBOL* sp;
            if ((sp = gsearch(in->name)) != NULL)
                preverror(ERR_VARIABLE_OBSCURES_VARIABLE_AT_HIGHER_SCOPE, in->name, sp->declfile, sp->declline);
        }
#if defined(PARSER_ONLY)
    if (AddName(in, table) && table != ccHash)
#else
    if (AddName(in, table))
#endif
    {
#if defined(CPREPROCESSOR) || defined(PARSER_ONLY)
        pperrorstr(ERR_DUPLICATE_IDENTIFIER, in->name);
#else
        if (!structLevel || !templateNestingCount)
        {
            SYMBOL* sym = search(in->name, table);
            if (!sym || !sym->wasUsing || !in->wasUsing)
                preverrorsym(ERR_DUPLICATE_IDENTIFIER, in, in->declfile, in->declline);
        }
#endif
    }
}
void insert(SYMBOL* in, HASHTABLE* table)
{
    if (table)
    {
#ifdef PARSER_ONLY
        if (table != defsyms && table != kwhash && table != ccHash)
            if (!in->parserSet)
            {
                in->parserSet = true;
                ccSetSymbol(in);
            }
#endif
        baseinsert(in, table);
    }
    else
    {
        diag("insert: cannot insert");
    }
}

void insertOverload(SYMBOL* in, HASHTABLE* table)
{

    if (cparams.prm_extwarning)
        if (in->storage_class == sc_parameter || in->storage_class == sc_auto || in->storage_class == sc_register)
        {
            SYMBOL* sp;
            if ((sp = gsearch(in->name)) != NULL)
                preverror(ERR_VARIABLE_OBSCURES_VARIABLE_AT_HIGHER_SCOPE, in->name, sp->declfile, sp->declline);
        }
    if (AddOverloadName(in, table))
    {
#ifdef CPREPROCESSOR
        pperrorstr(ERR_DUPLICATE_IDENTIFIER, in->name);
#else
        SetLinkerNames(in, lk_cdecl);
        preverrorsym(ERR_DUPLICATE_IDENTIFIER, in, in->declfile, in->declline);
#endif
    }
}
