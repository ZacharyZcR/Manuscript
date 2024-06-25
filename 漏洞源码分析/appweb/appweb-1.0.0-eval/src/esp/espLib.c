/*
 * Embedthis ESP Library Source
 */
#include "esp.h"

#if ME_COM_ESP

/*
    edi.c -- Embedded Database Interface (EDI)

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/


#include    "pcre.h"

/************************************* Local **********************************/

static void addValidations(void);
static void formatFieldForJson(MprBuf *buf, EdiField *fp, int flags);
static void manageEdiService(EdiService *es, int flags);
static void manageEdiGrid(EdiGrid *grid, int flags);
static bool validateField(Edi *edi, EdiRec *rec, cchar *columnName, cchar *value);

/************************************* Code ***********************************/

PUBLIC EdiService *ediCreateService()
{
    EdiService      *es;

    if ((es = mprAllocObj(EdiService, manageEdiService)) == 0) {
        return 0;
    }
    MPR->ediService = es;
    es->providers = mprCreateHash(0, MPR_HASH_STATIC_VALUES | MPR_HASH_STABLE);
    addValidations();
    return es;
}


static void manageEdiService(EdiService *es, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(es->providers);
        mprMark(es->validations);
    }
}


PUBLIC int ediAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    mprRemoveKey(edi->schemaCache, tableName);
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->addColumn(edi, tableName, columnName, type, flags);
}


PUBLIC int ediAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->addIndex(edi, tableName, columnName, indexName);
}


PUBLIC void ediAddProvider(EdiProvider *provider)
{
    EdiService  *es;

    es = MPR->ediService;
    mprAddKey(es->providers, provider->name, provider);
}


static EdiProvider *lookupProvider(cchar *providerName)
{
    EdiService  *es;

    es = MPR->ediService;
    return mprLookupKey(es->providers, providerName);
}


PUBLIC int ediAddTable(Edi *edi, cchar *tableName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->addTable(edi, tableName);
}


static void manageValidation(EdiValidation *vp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(vp->name);
        mprMark(vp->data);
    }
}


PUBLIC int ediAddValidation(Edi *edi, cchar *name, cchar *tableName, cchar *columnName, cvoid *data)
{
    EdiService          *es;
    EdiValidation       *prior, *vp;
    MprList             *validations;
    cchar               *errMsg, *vkey;
    int                 column, next;

    es = MPR->ediService;
    if ((vp = mprAllocObj(EdiValidation, manageValidation)) == 0) {
        return MPR_ERR_MEMORY;
    }
    vp->name = sclone(name);
    if ((vp->vfn = mprLookupKey(es->validations, name)) == 0) {
        mprLog("error esp edi", 0, "Cannot find validation '%s'", name);
        return MPR_ERR_CANT_FIND;
    }
    if (smatch(name, "format") || smatch(name, "banned")) {
        if (!data || ((char*) data)[0] == '\0') {
            mprLog("error esp edi", 0, "Bad validation format pattern for %s", name);
            return MPR_ERR_BAD_SYNTAX;
        }
        if ((vp->mdata = pcre_compile2(data, 0, 0, &errMsg, &column, NULL)) == 0) {
            mprLog("error esp edi", 0, "Cannot compile validation pattern. Error %s at column %d", errMsg, column);
            return MPR_ERR_BAD_SYNTAX;
        }
        data = 0;
    }
    vp->data = data;
    vkey = sfmt("%s.%s", tableName, columnName);

    lock(edi);
    if ((validations = mprLookupKey(edi->validations, vkey)) == 0) {
        validations = mprCreateList(0, MPR_LIST_STABLE);
        mprAddKey(edi->validations, vkey, validations);
    }
    for (ITERATE_ITEMS(validations, prior, next)) {
        if (prior->vfn == vp->vfn) {
            break;
        }
    }
    if (!prior) {
        mprAddItem(validations, vp);
    }
    unlock(edi);
    return 0;
}


static bool validateField(Edi *edi, EdiRec *rec, cchar *columnName, cchar *value)
{
    EdiValidation   *vp;
    MprList         *validations;
    cchar           *error, *vkey;
    int             next;
    bool            pass;

    assert(edi);
    assert(rec);
    assert(columnName && *columnName);

    pass = 1;
    vkey = sfmt("%s.%s", rec->tableName, columnName);
    if ((validations = mprLookupKey(edi->validations, vkey)) != 0) {
        for (ITERATE_ITEMS(validations, vp, next)) {
            if ((error = (*vp->vfn)(vp, rec, columnName, value)) != 0) {
                if (rec->errors == 0) {
                    rec->errors = mprCreateHash(0, MPR_HASH_STABLE);
                }
                mprAddKey(rec->errors, columnName, sfmt("%s %s", columnName, error));
                pass = 0;
            }
        }
    }
    return pass;
}


PUBLIC int ediChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    mprRemoveKey(edi->schemaCache, tableName);
    return edi->provider->changeColumn(edi, tableName, columnName, type, flags);
}


PUBLIC void ediClose(Edi *edi)
{
    if (!edi || !edi->provider || !edi->path) {
        return;
    }
    edi->provider->close(edi);
    edi->path = NULL;
}


/*
    Create a record based on the table's schema. Not saved to the database.
 */
PUBLIC EdiRec *ediCreateRec(Edi *edi, cchar *tableName)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->createRec(edi, tableName);
}


PUBLIC int ediDelete(Edi *edi, cchar *path)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->deleteDatabase(path);
}


PUBLIC void ediDumpGrid(cchar *message, EdiGrid *grid)
{
    mprLog("info esp edi", 0, "%s: Grid: %s\nschema: %s,\ndata: %s", message, grid->tableName,
        ediGetTableSchemaAsJson(grid->edi, grid->tableName), ediGridAsJson(grid, MPR_JSON_PRETTY));
}


PUBLIC void ediDumpRec(cchar *message, EdiRec *rec)
{
    mprLog("info esp edi", 0, "%s: Rec: %s", message, ediRecAsJson(rec, MPR_JSON_PRETTY));
}


PUBLIC EdiGrid *ediFilterGridFields(EdiGrid *grid, cchar *fields, int include)
{
    EdiRec      *first, *rec;
    MprList     *fieldList;
    int         f, r, inlist;

    if (!grid || grid->nrecords == 0) {
        return grid;
    }
    first = grid->records[0];
    fieldList = mprCreateListFromWords(fields);

    /* Process list of fields to remove from the grid */
    for (f = 0; f < first->nfields; f++) {
        inlist = mprLookupStringItem(fieldList, first->fields[f].name) >= 0;
        if ((inlist && !include) || (!inlist && include)) {
            for (r = 0; r < grid->nrecords; r++) {
                rec = grid->records[r];
                memmove(&rec->fields[f], &rec->fields[f+1], (rec->nfields - f - 1) * sizeof(EdiField));
                rec->nfields--;
                /* Ensure never saved to database */
                rec->id = 0;
            }
            f--;
        }
    }
    return grid;
}


PUBLIC EdiRec *ediFilterRecFields(EdiRec *rec, cchar *fields, int include)
{
    MprList     *fieldList;
    int         f, inlist;

    if (rec == 0 || rec->nfields == 0) {
        return rec;
    }
    fieldList = mprCreateListFromWords(fields);

    for (f = 0; f < rec->nfields; f++) {
        inlist = mprLookupStringItem(fieldList, rec->fields[f].name) >= 0;
        if ((inlist && !include) || (!inlist && include)) {
            memmove(&rec->fields[f], &rec->fields[f+1], (rec->nfields - f - 1) * sizeof(EdiField));
            rec->nfields--;
            /* Ensure never saved to database */
            rec->id = 0;
            f--;
        }
    }
    /*
        Ensure never saved to database
     */
    rec->id = 0;
    return rec;
}


PUBLIC MprList *ediGetColumns(Edi *edi, cchar *tableName)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->getColumns(edi, tableName);
}


PUBLIC int ediGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->getColumnSchema(edi, tableName, columnName, type, flags, cid);
}


PUBLIC EdiField *ediGetNextField(EdiRec *rec, EdiField *fp, int offset)
{
    if (rec == 0 || rec->nfields <= 0) {
        return 0;
    }
    if (fp == 0) {
        if (offset >= rec->nfields) {
            return 0;
        }
        fp = &rec->fields[offset];
    } else {
        if (++fp >= &rec->fields[rec->nfields]) {
            fp = 0;
        }
    }
    return fp;
}


PUBLIC EdiRec *ediGetNextRec(EdiGrid *grid, EdiRec *rec)
{
    int     index;

    if (grid == 0 || grid->nrecords <= 0) {
        return 0;
    }
    if (rec == 0) {
        rec = grid->records[0];
        rec->index = 0;
    } else {
        index = rec->index + 1;
        if (index >= grid->nrecords) {
            rec = 0;
        } else {
            rec = grid->records[index];
            rec->index = index;
        }
    }
    return rec;
}


PUBLIC cchar *ediGetTableSchemaAsJson(Edi *edi, cchar *tableName)
{
    MprBuf      *buf;
    MprList     *columns;
    cchar       *schema, *s;
    int         c, type, flags, cid, ncols, next;

    if (tableName == 0 || *tableName == '\0') {
        return 0;
    }
    if ((schema = mprLookupKey(edi->schemaCache, tableName)) != 0) {
        return schema;
    }
    buf = mprCreateBuf(0, 0);
    ediGetTableDimensions(edi, tableName, NULL, &ncols);
    columns = ediGetColumns(edi, tableName);
    mprPutStringToBuf(buf, "{\n    \"types\": {\n");
    for (c = 0; c < ncols; c++) {
        ediGetColumnSchema(edi, tableName, mprGetItem(columns, c), &type, &flags, &cid);
        mprPutToBuf(buf, "      \"%s\": {\n        \"type\": \"%s\"\n      },\n",
            (char*) mprGetItem(columns, c), ediGetTypeString(type));
    }
    if (ncols > 0) {
        mprAdjustBufEnd(buf, -2);
    }
    mprRemoveItemAtPos(columns, 0);
    mprPutStringToBuf(buf, "\n    },\n    \"columns\": [ ");
    for (ITERATE_ITEMS(columns, s, next)) {
        mprPutToBuf(buf, "\"%s\", ", s);
    }
    if (columns->length > 0) {
        mprAdjustBufEnd(buf, -2);
    }
    mprPutStringToBuf(buf, " ]\n  }");
    mprAddNullToBuf(buf);
    schema = mprGetBufStart(buf);
    mprAddKey(edi->schemaCache, tableName, schema);
    return schema;
}


PUBLIC cchar *ediGetGridSchemaAsJson(EdiGrid *grid)
{
    if (grid) {
        return ediGetTableSchemaAsJson(grid->edi, grid->tableName);
    }
    return 0;
}


PUBLIC cchar *ediGetRecSchemaAsJson(EdiRec *rec)
{
    if (rec) {
        return ediGetTableSchemaAsJson(rec->edi, rec->tableName);
    }
    return 0;
}


PUBLIC MprHash *ediGetRecErrors(EdiRec *rec)
{
    return rec->errors;
}


PUBLIC MprList *ediGetGridColumns(EdiGrid *grid)
{
    MprList     *cols;
    EdiRec      *rec;
    EdiField    *fp;

    cols = mprCreateList(0, 0);
    rec = grid->records[0];
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        mprAddItem(cols, fp->name);
    }
    return cols;
}


PUBLIC EdiField *ediGetField(EdiRec *rec, cchar *fieldName)
{
    EdiField    *fp;

    if (rec == 0) {
        return 0;
    }
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            return fp;
        }
    }
    return 0;
}


PUBLIC cchar *ediGetFieldValue(EdiRec *rec, cchar *fieldName)
{
    EdiField    *fp;

    if (rec == 0) {
        return 0;
    }
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            return fp->value;
        }
    }
    return 0;
}


PUBLIC int ediGetFieldType(EdiRec *rec, cchar *fieldName)
{
    int     type;

    if (ediGetColumnSchema(rec->edi, rec->tableName, fieldName, &type, NULL, NULL) < 0) {
        return 0;
    }
    return type;
}


PUBLIC MprList *ediGetTables(Edi *edi)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->getTables(edi);
}


PUBLIC int ediGetTableDimensions(Edi *edi, cchar *tableName, int *numRows, int *numCols)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->getTableDimensions(edi, tableName, numRows, numCols);
}


PUBLIC char *ediGetTypeString(int type)
{
    switch (type) {
    case EDI_TYPE_BINARY:
        return "binary";
    case EDI_TYPE_BOOL:
        return "bool";
    case EDI_TYPE_DATE:
        return "date";
    case EDI_TYPE_FLOAT:
        return "float";
    case EDI_TYPE_INT:
        return "int";
    case EDI_TYPE_STRING:
        return "string";
    case EDI_TYPE_TEXT:
        return "text";
    }
    return 0;
}


PUBLIC cchar *ediGridAsJson(EdiGrid *grid, int flags)
{
    EdiRec      *rec;
    EdiField    *fp;
    MprBuf      *buf;
    bool        pretty;
    int         r, f;

    pretty = flags & MPR_JSON_PRETTY;
    buf = mprCreateBuf(0, 0);
    mprPutStringToBuf(buf, "[");
    if (grid) {
        if (pretty) mprPutCharToBuf(buf, '\n');
        for (r = 0; r < grid->nrecords; r++) {
            if (pretty) mprPutStringToBuf(buf, "    ");
            mprPutStringToBuf(buf, "{");
            rec = grid->records[r];
            for (f = 0; f < rec->nfields; f++) {
                fp = &rec->fields[f];
                mprFormatJsonName(buf, fp->name, MPR_JSON_QUOTES);
                if (pretty) {
                    mprPutStringToBuf(buf, ": ");
                } else {
                    mprPutCharToBuf(buf, ':');
                }
                formatFieldForJson(buf, fp, flags);
                if ((f+1) < rec->nfields) {
                    mprPutStringToBuf(buf, ",");
                }
            }
            mprPutStringToBuf(buf, "}");
            if ((r+1) < grid->nrecords) {
                mprPutCharToBuf(buf, ',');
            }
            if (pretty) mprPutCharToBuf(buf, '\n');
        }
    }
    mprPutStringToBuf(buf, "]");
    if (pretty) mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);
    return mprGetBufStart(buf);
}


PUBLIC int ediLoad(Edi *edi, cchar *path)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->load(edi, path);
}


PUBLIC int ediLookupField(Edi *edi, cchar *tableName, cchar *fieldName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->lookupField(edi, tableName, fieldName);
}


PUBLIC EdiProvider *ediLookupProvider(cchar *providerName)
{
    return lookupProvider(providerName);
}


PUBLIC Edi *ediOpen(cchar *path, cchar *providerName, int flags)
{
    EdiProvider     *provider;
    Edi             *edi;

    if ((provider = lookupProvider(providerName)) == 0) {
        mprLog("error esp edi", 0, "Cannot find EDI provider '%s'", providerName);
        return 0;
    }
    if ((edi = provider->open(path, flags)) != 0) {
        edi->path = sclone(path);
    }
    return edi;
}


PUBLIC Edi *ediClone(Edi *edi)
{
    Edi     *cloned;

    if (!edi || !edi->provider) {
        return 0;
    }
    if ((cloned = edi->provider->open(edi->path, edi->flags)) != 0) {
        cloned->validations = edi->validations;
    }
    return cloned;
}


PUBLIC EdiGrid *ediQuery(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list vargs)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->query(edi, cmd, argc, argv, vargs);
}


PUBLIC cchar *ediReadFieldValue(Edi *edi, cchar *fmt, cchar *tableName, cchar *key, cchar *columnName, cchar *defaultValue)
{
    EdiField    field;

    field = ediReadField(edi, tableName, key, columnName);
    if (!field.valid) {
        return defaultValue;
    }
    return field.value;
}


PUBLIC EdiField ediReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    EdiField    field;

    if (!edi || !edi->provider) {
        memset(&field, 0, sizeof(EdiField));
        return field;
    }
    return edi->provider->readField(edi, tableName, key, fieldName);
}


PUBLIC EdiGrid *ediFindGrid(Edi *edi, cchar *tableName, cchar *select)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->findGrid(edi, tableName, select);
}


PUBLIC EdiRec *ediFindRec(Edi *edi, cchar *tableName, cchar *select)
{
    EdiGrid     *grid;

    if (!edi || !edi->provider) {
        return 0;
    }
    if ((grid = edi->provider->findGrid(edi, tableName, select)) == 0) {
        return 0;

    }
    if (grid->nrecords > 0) {
        return grid->records[0];
    }
    return 0;
}


PUBLIC EdiRec *ediReadRec(Edi *edi, cchar *tableName, cchar *key)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->readRec(edi, tableName, key);
}


#if DEPRECATED
PUBLIC EdiRec *ediFindRecWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    EdiGrid *grid;

    if ((grid = ediReadWhere(edi, tableName, fieldName, operation, value)) == 0) {
        return 0;
    }
    if (grid->nrecords > 0) {
        return grid->records[0];
    }
    return 0;
}


PUBLIC EdiGrid *ediReadWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->findGrid(edi, tableName, sfmt("%s %s %s", fieldName, operation, value));
}


PUBLIC EdiGrid *ediReadTable(Edi *edi, cchar *tableName)
{
    if (!edi || !edi->provider) {
        return 0;
    }
    return edi->provider->findGrid(edi, tableName, NULL);
}
#endif


PUBLIC cchar *ediRecAsJson(EdiRec *rec, int flags)
{
    MprBuf      *buf;
    EdiField    *fp;
    bool        pretty;
    int         f;

    pretty = flags & MPR_JSON_PRETTY;
    buf = mprCreateBuf(0, 0);
    mprPutStringToBuf(buf, "{ ");
    if (rec) {
        for (f = 0; f < rec->nfields; f++) {
            fp = &rec->fields[f];
            mprFormatJsonName(buf, fp->name, MPR_JSON_QUOTES);
            if (pretty) {
                mprPutStringToBuf(buf, ": ");
            } else {
                mprPutCharToBuf(buf, ':');
            }
            formatFieldForJson(buf, fp, flags);
            if ((f+1) < rec->nfields) {
                mprPutStringToBuf(buf, ",");
            }
        }
    }
    mprPutStringToBuf(buf, "}");
    if (pretty) mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);
    return mprGetBufStart(buf);;
}


PUBLIC int edRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    mprRemoveKey(edi->schemaCache, tableName);
    return edi->provider->removeColumn(edi, tableName, columnName);
}


PUBLIC int ediRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->removeIndex(edi, tableName, indexName);
}


#if KEEP
PUBLIC int ediRemoveRec(Edi *edi, cchar *tableName, cchar *query)
{
    EdiRec  *rec;

    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    if ((rec = ediFindRec(edi, tableName, query)) == 0) {
        return MPR_ERR_CANT_READ;
    }
    return edi->provider->removeRecByKey(edi, tableName, rec->id);
}
#endif


PUBLIC int ediRemoveRec(Edi *edi, cchar *tableName, cchar *key)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->removeRec(edi, tableName, key);
}


PUBLIC int ediRemoveTable(Edi *edi, cchar *tableName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->removeTable(edi, tableName);
}


PUBLIC int ediRenameTable(Edi *edi, cchar *tableName, cchar *newTableName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    mprRemoveKey(edi->schemaCache, tableName);
    return edi->provider->renameTable(edi, tableName, newTableName);
}


PUBLIC int ediRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    mprRemoveKey(edi->schemaCache, tableName);
    return edi->provider->renameColumn(edi, tableName, columnName, newColumnName);
}


PUBLIC int ediSave(Edi *edi)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    if (edi->flags & EDI_PRIVATE) {
        /* Skip saving for in-memory private databases */
        return 0;
    }
    return edi->provider->save(edi);
}


PUBLIC int ediUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->updateField(edi, tableName, key, fieldName, value);
}


PUBLIC int ediUpdateFieldFmt(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *fmt, ...)
{
    va_list     ap;
    cchar       *value;

    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    va_start(ap, fmt);
    value = sfmtv(fmt, ap);
    va_end(ap);
    return edi->provider->updateField(edi, tableName, key, fieldName, value);
}


PUBLIC int ediUpdateRec(Edi *edi, EdiRec *rec)
{
    if (!edi || !edi->provider) {
        return MPR_ERR_BAD_STATE;
    }
    return edi->provider->updateRec(edi, rec);
}


PUBLIC bool ediValidateRec(EdiRec *rec)
{
    EdiField    *fp;
    bool        pass;
    int         c;

    assert(rec->edi);
    if (rec == 0 || rec->edi == 0) {
        return 0;
    }
    pass = 1;
    for (c = 0; c < rec->nfields; c++) {
        fp = &rec->fields[c];
        if (!validateField(rec->edi, rec, fp->name, fp->value)) {
            pass = 0;
            /* Keep going */
        }
    }
    return pass;
}


/********************************* Convenience *****************************/

/*
    Create a free-standing grid. Not saved to the database
    The edi and tableName parameters can be null
 */
PUBLIC EdiGrid *ediCreateBareGrid(Edi *edi, cchar *tableName, int nrows)
{
    EdiGrid  *grid;

    if ((grid = mprAllocBlock(sizeof(EdiGrid) + sizeof(EdiRec*) * nrows, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(grid, (MprManager) manageEdiGrid);
    grid->nrecords = nrows;
    grid->edi = edi;
    grid->tableName = tableName? sclone(tableName) : 0;
    return grid;
}


/*
    Create a free-standing record. Not saved to the database.
    The tableName parameter can be null. The fields are not initialized (no schema).
 */
PUBLIC EdiRec *ediCreateBareRec(Edi *edi, cchar *tableName, int nfields)
{
    EdiRec      *rec;

    if ((rec = mprAllocBlock(sizeof(EdiRec) + sizeof(EdiField) * nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(rec, (MprManager) ediManageEdiRec);
    rec->edi = edi;
    rec->tableName = sclone(tableName);
    rec->nfields = nfields;
    return rec;
}


PUBLIC cchar *ediFormatField(cchar *fmt, EdiField *fp)
{
    MprTime     when;

    if (fp->value == 0) {
        return "null";
    }
    switch (fp->type) {
    case EDI_TYPE_BINARY:
    case EDI_TYPE_BOOL:
        return fp->value;

    case EDI_TYPE_DATE:
        if (fmt == 0) {
            fmt = MPR_DEFAULT_DATE;
        }
        if (mprParseTime(&when, fp->value, MPR_UTC_TIMEZONE, 0) == 0) {
            return mprFormatLocalTime(fmt, when);
        }
        return fp->value;

    case EDI_TYPE_FLOAT:
        if (fmt == 0) {
            return fp->value;
        }
        return sfmt(fmt, atof(fp->value));

    case EDI_TYPE_INT:
        if (fmt == 0) {
            return fp->value;
        }
        return sfmt(fmt, stoi(fp->value));

    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
        if (fmt == 0) {
            return fp->value;
        }
        return sfmt(fmt, fp->value);

    default:
        mprLog("error esp edi", 0, "Unknown field type %d", fp->type);
    }
    return 0;
}


static void formatFieldForJson(MprBuf *buf, EdiField *fp, int flags)
{
    MprTime     when;
    cchar       *value;

    value = fp->value;

    if (value == 0) {
        mprPutStringToBuf(buf, "null");
        return;
    }
    switch (fp->type) {
    case EDI_TYPE_BINARY:
        mprPutToBuf(buf, "-binary-");
        return;

    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
        mprFormatJsonValue(buf, MPR_JSON_STRING, fp->value, 0);
        return;

    case EDI_TYPE_BOOL:
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
        mprPutStringToBuf(buf, fp->value);
        return;

    case EDI_TYPE_DATE:
        if (flags & MPR_JSON_ENCODE_TYPES) {
            if (mprParseTime(&when, fp->value, MPR_UTC_TIMEZONE, 0) == 0) {
                mprPutToBuf(buf, "\"{type:date}%s\"", mprFormatUniversalTime(MPR_ISO_DATE, when));
            } else {
                mprPutToBuf(buf, "%s", fp->value);
            }
        } else {
            mprPutToBuf(buf, "%s", fp->value);
        }
        return;

    default:
        mprLog("error esp edi", 0, "Unknown field type %d", fp->type);
        mprPutStringToBuf(buf, "null");
    }
}


typedef struct Col {
    EdiGrid     *grid;          /* Source grid for this column */
    EdiField    *fp;            /* Field definition for this column */
    int         joinField;      /* Foreign key field index */
    int         field;          /* Field index in the foreign table */
} Col;


/*
    Create a list of columns to use for a joined table
    For all foreign key columns (ends with "Id"), join the columns from the referenced table.
 */
static MprList *joinColumns(MprList *cols, EdiGrid *grid, MprHash *grids, int joinField, int follow)
{
    EdiGrid     *foreignGrid;
    EdiRec      *rec;
    EdiField    *fp;
    Col         *col;
    cchar       *tableName;

    if (grid->nrecords == 0) {
        return cols;
    }
    rec = grid->records[0];
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
#if KEEP
        if (fp->flags & EDI_FOREIGN && follow)
#else
        if (sends(fp->name, "Id") && follow)
#endif
        {
            tableName = strim(fp->name, "Id", MPR_TRIM_END);
            if (!(foreignGrid = mprLookupKey(grids, tableName))) {
                col = mprAllocObj(Col, 0);
                col->grid = grid;
                col->field = (int) (fp - rec->fields);
                col->joinField = joinField;
                col->fp = fp;
                mprAddItem(cols, col);
            } else {
                cols = joinColumns(cols, foreignGrid, grids, (int) (fp - rec->fields), 0);
            }
        } else {
#if 0
            if (fp->flags & EDI_KEY && joinField >= 0) {
                /* Don't include ID fields from joined tables */
                continue;
            }
#endif
            col = mprAllocObj(Col, 0);
            col->grid = grid;
            col->field = (int) (fp - rec->fields);
            col->joinField = joinField;
            col->fp = fp;
            mprAddItem(cols, col);
        }
    }
    return cols;
}


/*
    Join grids using an INNER JOIN. All rows are returned. List of grids to join must be null terminated.
 */
PUBLIC EdiGrid *ediJoin(Edi *edi, ...)
{
    EdiGrid     *primary, *grid, *result, *current;
    EdiRec      *rec;
    EdiField    *dest, *fp;
    MprList     *cols, *rows;
    MprHash     *grids;
    Col         *col;
    va_list     vgrids;
    cchar       *keyValue;
    int         r, next, nfields, nrows;

    va_start(vgrids, edi);
    if ((primary = va_arg(vgrids, EdiGrid*)) == 0) {
        return 0;
    }
    if (primary->nrecords == 0) {
        return ediCreateBareGrid(edi, NULL, 0);
    }
    /*
        Build list of grids to join
     */
    grids = mprCreateHash(0, MPR_HASH_STABLE);
    for (;;) {
        if ((grid = va_arg(vgrids, EdiGrid*)) == 0) {
            break;
        }
        mprAddKey(grids, grid->tableName, grid);
    }
    va_end(vgrids);

    /*
        Get list of columns for the result. Each col object records the target grid and field index.
     */
    cols = joinColumns(mprCreateList(0, 0), primary, grids, -1, 1);
    nfields = mprGetListLength(cols);
    rows = mprCreateList(0, 0);

    for (r = 0; r < primary->nrecords; r++) {
        if ((rec = ediCreateBareRec(edi, NULL, nfields)) == 0) {
            assert(0);
            return 0;
        }
        mprAddItem(rows, rec);
        dest = rec->fields;
        current = 0;
        for (ITERATE_ITEMS(cols, col, next)) {
            if (col->grid == primary) {
                *dest = primary->records[r]->fields[col->field];
            } else {
                if (col->grid != current) {
                    current = col->grid;
                    keyValue = primary->records[r]->fields[col->joinField].value;
                    rec = ediReadRec(edi, col->grid->tableName, keyValue);
                }
                if (rec) {
                    fp = &rec->fields[col->field];
                    *dest = *fp;
                    dest->name = sfmt("%s.%s", col->grid->tableName, fp->name);
                } else {
                    dest->name = sclone("UNKNOWN");
                }
            }
            dest++;
        }
    }
    nrows = mprGetListLength(rows);
    if ((result = ediCreateBareGrid(edi, NULL, nrows)) == 0) {
        return 0;
    }
    for (r = 0; r < nrows; r++) {
        result->records[r] = mprGetItem(rows, r);
    }
    result->nrecords = nrows;
    result->count = nrows;
    return result;
}


PUBLIC void ediManageEdiRec(EdiRec *rec, int flags)
{
    int     fid;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(rec->edi);
        mprMark(rec->errors);
        mprMark(rec->tableName);
        mprMark(rec->id);

        for (fid = 0; fid < rec->nfields; fid++) {
            mprMark(rec->fields[fid].value);
            mprMark(rec->fields[fid].name);
        }
    }
}


static void manageEdiGrid(EdiGrid *grid, int flags)
{
    int     r;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(grid->edi);
        mprMark(grid->tableName);
        for (r = 0; r < grid->nrecords; r++) {
            mprMark(grid->records[r]);
        }
    }
}


/*
    grid = ediMakeGrid("[ \
        { id: '1', country: 'Australia' }, \
        { id: '2', country: 'China' }, \
    ]");
 */
PUBLIC EdiGrid *ediMakeGrid(cchar *json)
{
    MprJson     *obj, *row, *rp, *cp;
    EdiGrid     *grid;
    EdiRec      *rec;
    EdiField    *fp;
    int         col, r, nrows, nfields;

    if ((obj = mprParseJson(json)) == 0) {
        assert(0);
        return 0;
    }
    nrows = (int) mprGetJsonLength(obj);
    if ((grid = ediCreateBareGrid(NULL, "", nrows)) == 0) {
        assert(0);
        return 0;
    }
    if (nrows <= 0) {
        return grid;
    }
    for (ITERATE_JSON(obj, rp, r)) {
        if (rp->type == MPR_JSON_VALUE) {
            nfields = 1;
            if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
                return 0;
            }
            fp = rec->fields;
            fp->valid = 1;
            fp->name = sclone("value");
            fp->value = rp->value;
            fp->type = EDI_TYPE_STRING;
            fp->flags = 0;
        } else {
            row = rp;
            nfields = (int) mprGetJsonLength(row);
            if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
                return 0;
            }
            fp = rec->fields;
            for (ITERATE_JSON(row, cp, col)) {
                if (fp >= &rec->fields[nfields]) {
                    break;
                }
                fp->valid = 1;
                fp->name = cp->name;
                fp->type = EDI_TYPE_STRING;
                fp->flags = 0;
                fp++;
            }
            if (ediSetFields(rec, row) == 0) {
                assert(0);
                return 0;
            }
        }
        grid->records[r] = rec;
    }
    return grid;
}


PUBLIC MprJson *ediMakeJson(cchar *fmt, ...)
{
    MprJson     *obj;
    va_list     args;

    va_start(args, fmt);
    obj = mprParseJson(sfmtv(fmt, args));
    va_end(args);
    return obj;
}


/*
    rec = ediMakeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
 */
PUBLIC EdiRec *ediMakeRec(cchar *json)
{
    MprHash     *obj;
    MprKey      *kp;
    EdiRec      *rec;
    EdiField    *fp;
    int         f, nfields;

    if ((obj = mprDeserialize(json)) == 0) {
        return 0;
    }
    nfields = mprGetHashLength(obj);
    if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
        return 0;
    }
    for (f = 0, ITERATE_KEYS(obj, kp)) {
        if (kp->type == MPR_JSON_ARRAY || kp->type == MPR_JSON_OBJ) {
            continue;
        }
        fp = &rec->fields[f++];
        fp->valid = 1;
        fp->name = kp->key;
        fp->value = kp->data;
        fp->type = EDI_TYPE_STRING;
        fp->flags = 0;
    }
    return rec;
}


PUBLIC EdiRec *ediMakeRecFromJson(cchar *tableName, MprJson *fields)
{
    MprJson     *field;
    EdiRec      *rec;
    EdiField    *fp;
    int         f, fid;

    if ((rec = ediCreateBareRec(NULL, tableName, (int) mprGetJsonLength(fields))) == 0) {
        return 0;
    }
    for (f = 0, ITERATE_JSON(fields, field, fid)) {
        print("Table %s column %s value %s", tableName, field->name, field->value);
        if ((field->type & MPR_JSON_VALUE) == 0) {
            mprLog("error dlc", 0, "Bad field type %x", field->type);
        }
        fp = &rec->fields[f++];
        fp->valid = 1;
        fp->name = field->name;
        fp->value = field->value;
        fp->type = field->type & MPR_JSON_DATA_TYPE;
        fp->flags = 0;
        if (smatch(field->name, "id")) {
            rec->id = field->value;
        }
    }
    return rec;
}


PUBLIC int ediParseTypeString(cchar *type)
{
    if (smatch(type, "binary")) {
        return EDI_TYPE_BINARY;
    } else if (smatch(type, "bool") || smatch(type, "boolean")) {
        return EDI_TYPE_BOOL;
    } else if (smatch(type, "date")) {
        return EDI_TYPE_DATE;
    } else if (smatch(type, "float") || smatch(type, "double") || smatch(type, "number")) {
        return EDI_TYPE_FLOAT;
    } else if (smatch(type, "int") || smatch(type, "integer") || smatch(type, "fixed")) {
        return EDI_TYPE_INT;
    } else if (smatch(type, "string")) {
        return EDI_TYPE_STRING;
    } else if (smatch(type, "text")) {
        return EDI_TYPE_TEXT;
    } else {
        return MPR_ERR_BAD_ARGS;
    }
}


/*
    Swap rows for columns. The key field for each record is set to the prior column name.
 */
PUBLIC EdiGrid *ediPivotGrid(EdiGrid *grid, int flags)
{
    EdiGrid     *result;
    EdiRec      *rec, *first;
    EdiField    *src, *fp;
    int         r, c, nfields, nrows;

    if (grid->nrecords == 0) {
        return grid;
    }
    first = grid->records[0];
    nrows = first->nfields;
    nfields = grid->nrecords;
    result = ediCreateBareGrid(grid->edi, grid->tableName, nrows);

    for (c = 0; c < nrows; c++) {
        result->records[c] = rec = ediCreateBareRec(grid->edi, grid->tableName, nfields);
        fp = rec->fields;
        rec->id = first->fields[c].name;
        for (r = 0; r < grid->nrecords; r++) {
            src = &grid->records[r]->fields[c];
            fp->valid = 1;
            fp->name = src->name;
            fp->type = src->type;
            fp->value = src->value;
            fp->flags = src->flags;
            fp++; src++;
        }
    }
    return result;
}

PUBLIC EdiGrid *ediCloneGrid(EdiGrid *grid)
{
    EdiGrid     *result;
    EdiRec      *rec;
    EdiField    *src, *dest;
    int         r, c;

    if (grid->nrecords == 0) {
        return grid;
    }
    result = ediCreateBareGrid(grid->edi, grid->tableName, grid->nrecords);
    for (r = 0; r < grid->nrecords; r++) {
        rec = ediCreateBareRec(grid->edi, grid->tableName, grid->records[r]->nfields);
        result->records[r] = rec;
        rec->id = grid->records[r]->id;
        src = grid->records[r]->fields;
        dest = rec->fields;
        for (c = 0; c < rec->nfields; c++) {
            dest->valid = 1;
            dest->name = src->name;
            dest->value = src->value;
            dest->type = src->type;
            dest->flags = src->flags;
            dest++; src++;
        }
    }
    return result;
}


static cchar *mapEdiValue(cchar *value, int type)
{
    MprTime     time;

    if (value == 0) {
        return value;
    }
    switch (type) {
    case EDI_TYPE_DATE:
        if (!snumber(value)) {
            mprParseTime(&time, value, MPR_UTC_TIMEZONE, 0);
            value = itos(time);
        }
        break;

    case EDI_TYPE_BOOL:
        if (smatch(value, "false")) {
            value = "0";
        } else if (smatch(value, "true")) {
            value = "1";
        }
        break;

    case EDI_TYPE_BINARY:
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
    default:
        break;
    }
    return sclone(value);
}


PUBLIC EdiRec *ediSetField(EdiRec *rec, cchar *fieldName, cchar *value)
{
    EdiField    *fp;

    if (rec == 0) {
        return 0;
    }
    if (fieldName == 0) {
        return 0;
    }
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            fp->value = mapEdiValue(value, fp->type);
            return rec;
        }
    }
    return rec;
}


PUBLIC EdiRec *ediSetFieldFmt(EdiRec *rec, cchar *fieldName, cchar *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    ediSetField(rec, fieldName, sfmtv(fmt, ap));
    va_end(ap);
    return rec;
}


PUBLIC EdiRec *ediSetFields(EdiRec *rec, MprJson *params)
{
    MprJson     *param;
    int         i;

    if (rec == 0) {
        return 0;
    }
    for (ITERATE_JSON(params, param, i)) {
        if (param->type & MPR_JSON_VALUE) {
            if (!ediSetField(rec, param->name, param->value)) {
                return 0;
            }
        }
    }
    return rec;
}


typedef struct GridSort {
    int     sortColumn;         /**< Column to sort on */
    int     sortOrder;          /**< Sort order: ascending == 1, descending == -1 */
} GridSort;

static int sortRec(EdiRec **r1, EdiRec **r2, GridSort *gs)
{
    EdiField    *f1, *f2;
    int64       v1, v2;

    f1 = &(*r1)->fields[gs->sortColumn];
    f2 = &(*r2)->fields[gs->sortColumn];
    if (f1->type == EDI_TYPE_INT) {
        v1 = stoi(f1->value);
        v2 = stoi(f2->value);
        if (v1 < v2) {
            return - gs->sortOrder;
        } else if (v1 > v2) {
            return gs->sortOrder;
        }
        return 0;
    } else {
        return scmp(f1->value, f2->value) * gs->sortOrder;
    }
}


static int lookupGridField(EdiGrid *grid, cchar *name)
{
    EdiRec      *rec;
    EdiField    *fp;

    if (grid->nrecords == 0) {
        return MPR_ERR_CANT_FIND;
    }
    rec = grid->records[0];
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(name, fp->name)) {
            return (int) (fp - rec->fields);
        }
    }
    return MPR_ERR_CANT_FIND;
}


PUBLIC EdiGrid *ediSortGrid(EdiGrid *grid, cchar *sortColumn, int sortOrder)
{
    GridSort    gs;

    if (grid->nrecords == 0) {
        return grid;
    }
    grid = ediCloneGrid(grid);
    gs.sortColumn = lookupGridField(grid, sortColumn);
    gs.sortOrder = sortOrder;
    mprSort(grid->records, grid->nrecords, sizeof(EdiRec*), (MprSortProc) sortRec, &gs);
    return grid;
}


/********************************* Validations *****************************/

static cchar *checkBoolean(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        if (scaselessmatch(value, "true") || scaselessmatch(value, "false")) {
            return 0;
        }
    }
    return "is not a number";
}


static cchar *checkDate(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    MprTime     time;

    if (value && *value) {
        if (mprParseTime(&time, value, MPR_UTC_TIMEZONE, NULL) < 0) {
            return 0;
        }
    }
    return "is not a date or time";
}


static cchar *checkFormat(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    int     matched[ME_MAX_ROUTE_MATCHES * 2];

    if (pcre_exec(vp->mdata, NULL, value, (int) slen(value), 0, 0, matched, sizeof(matched) / sizeof(int)) > 0) {
        return 0;
    }
    return "is in the wrong format";
}


static cchar *checkBanned(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    int     matched[ME_MAX_ROUTE_MATCHES * 2];

    if (pcre_exec(vp->mdata, NULL, value, (int) slen(value), 0, 0, matched, sizeof(matched) / sizeof(int)) > 0) {
        return "contains banned content";
    }
    return 0;
}


static cchar *checkInteger(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        if (snumber(value)) {
            return 0;
        }
    }
    return "is not an integer";
}


static cchar *checkNumber(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        if (strspn(value, "1234567890+-.") == strlen(value)) {
            return 0;
        }
    }
    return "is not a number";
}


static cchar *checkPresent(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        return 0;
    }
    return "is missing";
}


static cchar *checkUnique(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    EdiRec  *other;

    if ((other = ediReadRec(rec->edi, rec->tableName, sfmt("%s == %s", fieldName, value))) == 0) {
        return 0;
    }
    if (smatch(other->id, rec->id)) {
        return 0;
    }
    return "is not unique";
}


PUBLIC void ediAddFieldError(EdiRec *rec, cchar *field, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    if (rec->errors == 0) {
        rec->errors = mprCreateHash(0, MPR_HASH_STABLE);
    }
    mprAddKey(rec->errors, field, sfmtv(fmt, args));
    va_end(args);
}


PUBLIC void ediDefineValidation(cchar *name, EdiValidationProc vfn)
{
    EdiService  *es;

    es = MPR->ediService;
    mprAddKey(es->validations, name, vfn);
}


PUBLIC void ediDefineMigration(Edi *edi, EdiMigration forw, EdiMigration back)
{
    edi->forw = forw;
    edi->back = back;
}


PUBLIC void ediSetPrivate(Edi *edi, bool on)
{
    edi->flags &= ~EDI_PRIVATE;
    if (on) {
        edi->flags |= EDI_PRIVATE;
    }
}


PUBLIC void ediSetReadonly(Edi *edi, bool on)
{
    edi->flags &= ~EDI_NO_SAVE;
    if (on) {
        edi->flags |= EDI_NO_SAVE;
    }
}


static void addValidations()
{
    EdiService  *es;

    es = MPR->ediService;
    /* Thread safe */
    es->validations = mprCreateHash(0, MPR_HASH_STATIC_VALUES);
    ediDefineValidation("boolean", checkBoolean);
    ediDefineValidation("format", checkFormat);
    ediDefineValidation("banned", checkBanned);
    ediDefineValidation("integer", checkInteger);
    ediDefineValidation("number", checkNumber);
    ediDefineValidation("present", checkPresent);
    ediDefineValidation("date", checkDate);
    ediDefineValidation("unique", checkUnique);
}


/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    espAbbrev.c -- ESP Abbreviated API

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/



#if ME_ESP_ABBREV
/*************************************** Code *********************************/

PUBLIC void addHeader(cchar *key, cchar *fmt, ...)
{
    va_list     args;
    cchar       *value;

    va_start(args, fmt);
    value = sfmtv(fmt, args);
    espAddHeaderString(getStream(), key, value);
    va_end(args);
}


PUBLIC void addParam(cchar *key, cchar *value)
{
    if (!param(key)) {
        setParam(key, value);
    }
}


#if KEEP
PUBLIC cchar *body(cchar *key)
{
    return espGetParam(getStream(), sfmt("body.%s", key), 0);
}
#endif


PUBLIC bool canUser(cchar *abilities, bool warn)
{
    HttpStream    *stream;

    stream = getStream();
    if (httpCanUser(stream, abilities)) {
        return 1;
    }
    if (warn) {
        setStatus(HTTP_CODE_UNAUTHORIZED);
        sendResult(feedback("error", "Access Denied. Insufficient Privilege."));
    }
    return 0;
}


PUBLIC EdiRec *createRec(cchar *tableName, MprJson *params)
{
    return setRec(setFields(ediCreateRec(getDatabase(), tableName), params));
}


#if DEPRECATED || 1
PUBLIC bool createRecFromParams(cchar *table)
{
    return updateRec(createRec(table, params(NULL)));
}
#endif


/*
    Return the session ID
 */
PUBLIC cchar *createSession()
{
    return espCreateSession(getStream());
}


/*
    Destroy a session and erase the session state data.
    This emits an expired Set-Cookie header to the browser to force it to erase the cookie.
 */
PUBLIC void destroySession()
{
    httpDestroySession(getStream());
}


PUBLIC void dontAutoFinalize()
{
    espSetAutoFinalizing(getStream(), 0);
}


PUBLIC void dumpParams(cchar *message)
{
    mprLog("info esp edi", 0, "%s: %s", message, mprJsonToString(params(NULL), MPR_JSON_PRETTY));
}


PUBLIC void dumpGrid(cchar *message, EdiGrid *grid)
{
    ediDumpGrid(message, grid);
}


PUBLIC void dumpRec(cchar *message, EdiRec *rec)
{
    ediDumpRec(message, rec);
}


PUBLIC bool feedback(cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFeedbackv(getStream(), kind, fmt, args);
    va_end(args);

    /*
        Return true if there is not an error feedback message
     */
    return getFeedback("error") == 0;
}


PUBLIC void finalize()
{
    espFinalize(getStream());
}


PUBLIC cchar *findParams()
{
    MprJson     *fields, *key;
    MprBuf      *buf;
    cchar       *filter, *limit, *offset;
    int         index;

    buf = mprCreateBuf(0, 0);

    if ((fields = params("fields")) != 0) {
        for (ITERATE_JSON(fields, key, index)) {
            mprPutToBuf(buf, "%s == %s", key->name, key->value);
            if ((index + 1) < fields->length) {
                mprPutStringToBuf(buf, " AND ");
            }
        }
    }
    if ((filter = param("options.filter")) != 0) {
        if (mprGetBufLength(buf) > 0) {
            mprPutStringToBuf(buf, " AND ");
        }
        mprPutToBuf(buf, "* >< %s", filter);
    }
    offset = param("options.offset");
    limit = param("options.limit");
    if (offset && limit) {
        mprPutToBuf(buf, " LIMIT %d, %d", (int) stoi(offset), (int) stoi(limit));
    }
    return mprBufToString(buf);
}


#if DEPRECATED
PUBLIC void flash(cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFeedbackv(getStream(), kind, fmt, args);
    va_end(args);
}
#endif


PUBLIC void flush()
{
    espFlush(getStream());
}


PUBLIC HttpAuth *getAuth()
{
    return espGetAuth(getStream());
}


PUBLIC MprList *getColumns(EdiRec *rec)
{
    if (rec == 0) {
        if ((rec = getRec()) == 0) {
            return 0;
        }
    }
    return ediGetColumns(getDatabase(), rec->tableName);
}


PUBLIC cchar *getConfig(cchar *field)
{
    HttpRoute   *route;
    cchar       *value;

    route = getStream()->rx->route;
    if ((value = mprGetJson(route->config, field)) == 0) {
        return "";
    }
    return value;
}


PUBLIC HttpStream *getStream()
{
    HttpStream    *stream;

    stream = mprGetThreadData(((Esp*) MPR->espService)->local);
    if (stream == 0) {
        mprLog("error esp", 0, "Stream is not defined in thread local storage.\n"
        "If using a callback, make sure you invoke espSetStream with the connection before using the ESP abbreviated API");
    }
    return stream;
}


PUBLIC cchar *getCookies()
{
    return espGetCookies(getStream());
}


PUBLIC MprOff getContentLength()
{
    return espGetContentLength(getStream());
}


PUBLIC cchar *getContentType()
{
    return getStream()->rx->mimeType;
}


PUBLIC void *getData()
{
    return espGetData(getStream());
}


PUBLIC Edi *getDatabase()
{
    return espGetDatabase(getStream());
}


PUBLIC MprDispatcher *getDispatcher()
{
    HttpStream    *stream;

    if ((stream = getStream()) == 0) {
        return 0;
    }
    return stream->dispatcher;
}


PUBLIC cchar *getDocuments()
{
    return getStream()->rx->route->documents;
}


PUBLIC EspRoute *getEspRoute()
{
    return espGetEspRoute(getStream());
}


PUBLIC cchar *getFeedback(cchar *kind)
{
    return espGetFeedback(getStream(), kind);
}


PUBLIC cchar *getField(EdiRec *rec, cchar *field)
{
    return ediGetFieldValue(rec, field);
}


PUBLIC cchar *getFieldError(cchar *field)
{
    return mprLookupKey(getRec()->errors, field);
}


PUBLIC EdiGrid *getGrid()
{
    return getStream()->grid;
}


PUBLIC cchar *getHeader(cchar *key)
{
    return espGetHeader(getStream(), key);
}


PUBLIC cchar *getMethod()
{
    return espGetMethod(getStream());
}


PUBLIC cchar *getQuery()
{
    return getStream()->rx->parsedUri->query;
}


PUBLIC EdiRec *getRec()
{
    return getStream()->record;
}


PUBLIC cchar *getReferrer()
{
    return espGetReferrer(getStream());
}


PUBLIC EspReq *getReq()
{
    return getStream()->data;
}


PUBLIC HttpRoute *getRoute()
{
    return espGetRoute(getStream());
}


PUBLIC cchar *getSecurityToken()
{
    return httpGetSecurityToken(getStream(), 0);
}


/*
    Get a session and return the session ID. Creates a session if one does not already exist.
 */
PUBLIC cchar *getSessionID()
{
    return espGetSessionID(getStream(), 1);
}


PUBLIC cchar *getSessionVar(cchar *key)
{
    return httpGetSessionVar(getStream(), key, 0);
}


PUBLIC cchar *getPath()
{
    return espGetPath(getStream());
}


PUBLIC MprList *getUploads()
{
    return espGetUploads(getStream());
}


PUBLIC cchar *getUri()
{
    return espGetUri(getStream());
}


PUBLIC bool hasGrid()
{
    return espHasGrid(getStream());
}


PUBLIC bool hasRec()
{
    return espHasRec(getStream());
}


PUBLIC int intParam(cchar *key)
{
    return (int) stoi(param(key));
}


PUBLIC bool isAuthenticated()
{
    return httpIsAuthenticated(getStream());
}

PUBLIC bool isEof()
{
    return httpIsEof(getStream());
}

PUBLIC bool isFinalized()
{
    return espIsFinalized(getStream());
}


PUBLIC bool isSecure()
{
    return espIsSecure(getStream());
}


PUBLIC EdiGrid *makeGrid(cchar *contents)
{
    return ediMakeGrid(contents);
}


PUBLIC MprHash *makeHash(cchar *fmt, ...)
{
    va_list     args;
    cchar       *str;

    va_start(args, fmt);
    str = sfmtv(fmt, args);
    va_end(args);
    return mprDeserialize(str);
}


PUBLIC MprJson *makeJson(cchar *fmt, ...)
{
    va_list     args;
    cchar       *str;

    va_start(args, fmt);
    str = sfmtv(fmt, args);
    va_end(args);
    return mprParseJson(str);
}


PUBLIC EdiRec *makeRec(cchar *contents)
{
    return ediMakeRec(contents);
}


PUBLIC cchar *makeUri(cchar *target)
{
    return espUri(getStream(), target);
}


PUBLIC cchar *md5(cchar *str)
{
    return mprGetMD5(str);
}


PUBLIC bool modeIs(cchar *kind)
{
    HttpRoute   *route;

    route = getStream()->rx->route;
    return smatch(route->mode, kind);
}


PUBLIC cchar *nonce()
{
    return mprGetMD5(itos(mprRandom()));
}


PUBLIC cchar *param(cchar *key)
{
    return espGetParam(getStream(), key, 0);
}


PUBLIC MprJson *params(cchar *var)
{
    if (var) {
        return mprGetJsonObj(espGetParams(getStream()), var);
    } else {
        return espGetParams(getStream());
    }
}


PUBLIC ssize receive(char *buf, ssize len)
{
    return httpRead(getStream(), buf, len);
}


PUBLIC EdiGrid *findGrid(cchar *tableName, cchar *select)
{
    return setGrid(ediFindGrid(getDatabase(), tableName, select));
}


PUBLIC EdiRec *findRec(cchar *tableName, cchar *select)
{
    return setRec(ediFindRec(getDatabase(), tableName, select));
}


PUBLIC EdiRec *readRec(cchar *tableName, cchar *key)
{
    if (key == 0 || *key == 0) {
        key = "1";
    }
    return setRec(ediReadRec(getDatabase(), tableName, key));
}


#if DEPRECATE
PUBLIC EdiRec *findRecWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return setRec(ediFindRecWhere(getDatabase(), tableName, fieldName, operation, value));
}


PUBLIC EdiGrid *readWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return setGrid(ediReadWhere(getDatabase(), tableName, fieldName, operation, value));
}


PUBLIC EdiGrid *readTable(cchar *tableName)
{
    return setGrid(ediReadTable(getDatabase(), tableName));
}
#endif


PUBLIC void redirect(cchar *target)
{
    espRedirect(getStream(), 302, target);
}


PUBLIC void redirectBack()
{
    espRedirectBack(getStream());
}


PUBLIC void removeCookie(cchar *name)
{
    espRemoveCookie(getStream(), name);
}


#if KEEP
PUBLIC bool removeRec(cchar *tableName, cchar *query)
{
    if (ediRemoveRec(getDatabase(), tableName, query) < 0) {
        feedback("error", "Cannot delete %s", stitle(tableName));
        return 0;
    }
    feedback("info", "Deleted %s", stitle(tableName));
    return 1;
}
#endif


PUBLIC bool removeRec(cchar *tableName, cchar *key)
{
    if (ediRemoveRec(getDatabase(), tableName, key) < 0) {
        feedback("error", "Cannot delete %s", stitle(tableName));
        return 0;
    }
    feedback("info", "Deleted %s", stitle(tableName));
    return 1;
}


PUBLIC void removeSessionVar(cchar *key)
{
    httpRemoveSessionVar(getStream(), key);
}


PUBLIC ssize render(cchar *fmt, ...)
{
    va_list     args;
    ssize       count;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    count = espRenderString(getStream(), msg);
    va_end(args);
    return count;
}


PUBLIC ssize renderCached()
{
    return espRenderCached(getStream());;
}


PUBLIC ssize renderConfig()
{
    return espRenderConfig(getStream());;
}


PUBLIC void renderError(int status, cchar *fmt, ...)
{
    va_list     args;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmt(fmt, args);
    espRenderError(getStream(), status, "%s", msg);
    va_end(args);
}


PUBLIC ssize renderFile(cchar *path)
{
    return espRenderFile(getStream(), path);
}


PUBLIC void renderFeedback(cchar *kind)
{
    espRenderFeedback(getStream(), kind);
}


PUBLIC ssize renderSafe(cchar *fmt, ...)
{
    va_list     args;
    ssize       count;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    count = espRenderSafeString(getStream(), msg);
    va_end(args);
    return count;
}


PUBLIC ssize renderString(cchar *s)
{
    return espRenderString(getStream(), s);
}


PUBLIC void renderView(cchar *view)
{
    espRenderDocument(getStream(), view);
}


PUBLIC int runCmd(cchar *command, char *input, char **output, char **error, MprTime timeout, int flags)
{
    MprCmd  *cmd;

    cmd = mprCreateCmd(getDispatcher());
    return mprRunCmd(cmd, command, NULL, input, output, error, timeout, MPR_CMD_IN  | MPR_CMD_OUT | MPR_CMD_ERR | flags);
}


/*
    Add a security token to the response. The token is generated as a HTTP header and session cookie.
 */
PUBLIC void securityToken()
{
    httpAddSecurityToken(getStream(), 0);
}


PUBLIC ssize sendGrid(EdiGrid *grid)
{
    return espSendGrid(getStream(), grid, 0);
}


PUBLIC ssize sendRec(EdiRec *rec)
{
    return espSendRec(getStream(), rec, 0);
}


PUBLIC void sendResult(bool status)
{
    espSendResult(getStream(), status);
}


PUBLIC void setStream(HttpStream *stream)
{
    espSetStream(stream);
}


PUBLIC void setContentType(cchar *mimeType)
{
    espSetContentType(getStream(), mimeType);
}


PUBLIC void setCookie(cchar *name, cchar *value, cchar *path, cchar *cookieDomain, MprTicks lifespan, bool isSecure)
{
    espSetCookie(getStream(), name, value, path, cookieDomain, lifespan, isSecure);
}


PUBLIC void setData(void *data)
{
    espSetData(getStream(), data);
}


PUBLIC EdiRec *setField(EdiRec *rec, cchar *fieldName, cchar *value)
{
    return ediSetField(rec, fieldName, value);
}


PUBLIC EdiRec *setFields(EdiRec *rec, MprJson *params)
{
    return ediSetFields(rec, params);
}


PUBLIC EdiGrid *setGrid(EdiGrid *grid)
{
    getStream()->grid = grid;
    return grid;
}


PUBLIC void setHeader(cchar *key, cchar *fmt, ...)
{
    va_list     args;
    cchar       *value;

    va_start(args, fmt);
    value = sfmtv(fmt, args);
    espSetHeaderString(getStream(), key, value);
    va_end(args);
}


PUBLIC void setIntParam(cchar *key, int value)
{
    espSetIntParam(getStream(), key, value);
}


PUBLIC void setNotifier(HttpNotifier notifier)
{
    espSetNotifier(getStream(), notifier);
}


PUBLIC void setParam(cchar *key, cchar *value)
{
    espSetParam(getStream(), key, value);
}


PUBLIC EdiRec *setRec(EdiRec *rec)
{
    return espSetRec(getStream(), rec);
}


PUBLIC void setSessionVar(cchar *key, cchar *value)
{
    httpSetSessionVar(getStream(), key, value);
}


PUBLIC void setStatus(int status)
{
    espSetStatus(getStream(), status);
}


PUBLIC cchar *session(cchar *key)
{
    return getSessionVar(key);
}


PUBLIC void setTimeout(void *proc, MprTicks timeout, void *data)
{
    mprCreateLocalEvent(getStream()->dispatcher, "setTimeout", (int) timeout, proc, data, 0);
}


PUBLIC void showRequest()
{
    espShowRequest(getStream());
}


PUBLIC EdiGrid *sortGrid(EdiGrid *grid, cchar *sortColumn, int sortOrder)
{
    return ediSortGrid(grid, sortColumn, sortOrder);
}


PUBLIC void updateCache(cchar *uri, cchar *data, int lifesecs)
{
    espUpdateCache(getStream(), uri, data, lifesecs);
}


PUBLIC bool updateField(cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    return ediUpdateField(getDatabase(), tableName, key, fieldName, value) == 0;
}


PUBLIC bool updateFields(cchar *tableName, MprJson *params)
{
    EdiRec  *rec;
    cchar   *key;

    key = mprReadJson(params, "id");
    if ((rec = ediSetFields(ediReadRec(getDatabase(), tableName, key), params)) == 0) {
        return 0;
    }
    return updateRec(rec);
}


PUBLIC bool updateRec(EdiRec *rec)
{
    if (!rec) {
        feedback("error", "Cannot save record");
        return 0;
    }
    setRec(rec);
    if (ediUpdateRec(getDatabase(), rec) < 0) {
        feedback("error", "Cannot save %s", stitle(rec->tableName));
        return 0;
    }
    feedback("info", "Saved %s", stitle(rec->tableName));
    return 1;
}


#if DEPRECATED
PUBLIC bool updateRecFromParams(cchar *table)
{
    return updateRec(setFields(findRec(table, param("id")), params(NULL)));
}
#endif


PUBLIC cchar *uri(cchar *target, ...)
{
    va_list     args;
    cchar       *uri;

    va_start(args, target);
    uri = sfmtv(target, args);
    va_end(args);
    return httpLink(getStream(), uri);
}


PUBLIC cchar *absuri(cchar *target, ...)
{
    va_list     args;
    cchar       *uri;

    va_start(args, target);
    uri = sfmtv(target, args);
    va_end(args);
    return httpLinkAbs(getStream(), uri);
}


#if DEPRECATED
/*
    <% stylesheets(patterns); %>

    Where patterns may contain *, ** and !pattern for exclusion
 */
PUBLIC void stylesheets(cchar *patterns)
{
    HttpStream  *stream;
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    MprList     *files;
    cchar       *filename, *ext, *uri, *path, *kind, *version, *clientDir;
    int         next;

    stream = getStream();
    rx = stream->rx;
    route = rx->route;
    eroute = route->eroute;
    patterns = httpExpandRouteVars(route, patterns);
    clientDir = httpGetDir(route, "documents");

    if (!patterns || !*patterns) {
        version = espGetConfig(route, "version", "1.0.0");
        if (eroute->combineSheet) {
            /* Previously computed combined stylesheet filename */
            stylesheets(eroute->combineSheet);

        } else if (espGetConfig(route, "http.content.combine[@=css]", 0)) {
            if (espGetConfig(route, "http.content.minify[@=css]", 0)) {
                eroute->combineSheet = sfmt("css/all-%s.min.css", version);
            } else {
                eroute->combineSheet = sfmt("css/all-%s.css", version);
            }
            stylesheets(eroute->combineSheet);

        } else {
            /*
                Not combining into a single stylesheet, so give priority to all.less over all.css if present
                Load a pure CSS incase some styles need to be applied before the lesssheet is parsed
             */
            ext = espGetConfig(route, "http.content.stylesheets", "css");
            filename = mprJoinPathExt("css/all", ext);
            path = mprJoinPath(clientDir, filename);
            if (mprPathExists(path, R_OK)) {
                stylesheets(filename);
            } else if (!smatch(ext, "less")) {
                path = mprJoinPath(clientDir, "css/all.less");
                if (mprPathExists(path, R_OK)) {
                    stylesheets("css/all.less");
                }
            }
        }
    } else {
        if (sends(patterns, "all.less")) {
            path = mprJoinPath(clientDir, "css/fix.css");
            if (mprPathExists(path, R_OK)) {
                stylesheets("css/fix.css");
            }
        }
        if ((files = mprGlobPathFiles(clientDir, patterns, MPR_PATH_RELATIVE)) == 0 || mprGetListLength(files) == 0) {
            files = mprCreateList(0, 0);
            mprAddItem(files, patterns);
        }
        for (ITERATE_ITEMS(files, path, next)) {
            path = sjoin("~/", strim(path, ".gz", MPR_TRIM_END), NULL);
            uri = httpLink(stream, path);
            kind = mprGetPathExt(path);
            if (smatch(kind, "css")) {
                espRender(stream, "<link rel='stylesheet' type='text/css' href='%s' />\n", uri);
            } else {
                espRender(stream, "<link rel='stylesheet/%s' type='text/css' href='%s' />\n", kind, uri);
            }
        }
    }
}


/*
    <% scripts(patterns); %>

    Where patterns may contain *, ** and !pattern for exclusion
 */
PUBLIC void scripts(cchar *patterns)
{
    HttpStream  *stream;
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    MprList     *files;
    MprJson     *cscripts, *script;
    cchar       *uri, *path, *version;
    int         next, ci;

    stream = getStream();
    rx = stream->rx;
    route = rx->route;
    eroute = route->eroute;
    patterns = httpExpandRouteVars(route, patterns);

    if (!patterns || !*patterns) {
        version = espGetConfig(route, "version", "1.0.0");
        if (eroute->combineScript) {
            scripts(eroute->combineScript);
        } else if (espGetConfig(route, "http.content.combine[@=js]", 0)) {
            if (espGetConfig(route, "http.content.minify[@=js]", 0)) {
                eroute->combineScript = sfmt("all-%s.min.js", version);
            } else {
                eroute->combineScript = sfmt("all-%s.js", version);
            }
            scripts(eroute->combineScript);
        } else {
            if ((cscripts = mprGetJsonObj(route->config, "client.scripts")) != 0) {
                for (ITERATE_JSON(cscripts, script, ci)) {
                    scripts(script->value);
                }
            }
        }
        return;
    }
    if ((files = mprGlobPathFiles(httpGetDir(route, "client"), patterns, MPR_PATH_RELATIVE)) == 0 ||
            mprGetListLength(files) == 0) {
        files = mprCreateList(0, 0);
        mprAddItem(files, patterns);
    }
    for (ITERATE_ITEMS(files, path, next)) {
        if (schr(path, '$')) {
            path = stemplateJson(path, route->config);
        }
        path = sjoin("~/", strim(path, ".gz", MPR_TRIM_END), NULL);
        uri = httpLink(stream, path);
        espRender(stream, "<script src='%s' type='text/javascript'></script>\n", uri);
    }
}
#endif

#endif /* ME_ESP_ABBREV */
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    espConfig.c -- ESP Configuration

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/*********************************** Includes *********************************/



/************************************* Locals *********************************/

#define ITERATE_CONFIG(route, obj, child, index) \
    index = 0, child = obj ? obj->children: 0; obj && index < obj->length && !route->error; child = child->next, index++
static void defineEnv(HttpRoute *route, cchar *key, cchar *value);

/************************************** Code **********************************/

static void loadApp(HttpRoute *parent, MprJson *prop)
{
    HttpRoute   *route;
    MprList     *files;
    cchar       *config, *prefix;
    int         next;

    if (prop->type & MPR_JSON_OBJ) {
        prefix = mprGetJson(prop, "prefix");
        config = mprGetJson(prop, "config");
        route = httpCreateInheritedRoute(parent);
        if (espInit(route, prefix, config) < 0) {
            httpParseError(route, "Cannot define ESP application at: %s", config);
            return;
        }
        httpFinalizeRoute(route);

    } else if (prop->type & MPR_JSON_STRING) {
        files = mprGlobPathFiles(".", prop->value, MPR_PATH_RELATIVE);
        for (ITERATE_ITEMS(files, config, next)) {
            route = httpCreateInheritedRoute(parent);
            prefix = mprGetPathBase(mprGetPathDir(mprGetAbsPath(config)));
            if (espInit(route, prefix, config) < 0) {
                httpParseError(route, "Cannot define ESP application at: %s", config);
                return;
            }
            httpFinalizeRoute(route);
        }
    }
}


static void parseEsp(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    httpParseAll(route, key, prop);

    /*
        Fix ups
     */
    if (route->flags & HTTP_ROUTE_UTILITY) {
        eroute->compile = 1;
        eroute->update = 1;
    }
    if (eroute->app) {
        if (!mprLookupStringItem(route->indexes, "index.esp")) {
            httpAddRouteIndex(route, "index.esp");
        }
        if (!mprLookupStringItem(route->indexes, "index.html")) {
            httpAddRouteIndex(route, "index.html");
        }
    }
}

/*
    app: {
        source: [
            'patterns/ *.c',
        ],
        tokens: [
            CFLAGS: '-DMY=42'
        ],
    }
 */
static void parseApp(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;

    if (!(prop->type & MPR_JSON_OBJ)) {
        if (prop->type & MPR_JSON_TRUE) {
            eroute->app = 1;
        }
    } else {
        eroute->app = 1;
    }
    if (eroute->app) {
        /*
            Set some defaults before parsing "esp". This permits user overrides.
         */
        httpSetRouteXsrf(route, 1);
        httpAddRouteHandler(route, "espHandler", "");
        eroute->keep = smatch(route->mode, "release") == 0;
        espSetDefaultDirs(route, eroute->app);
    }
    if (prop->type & MPR_JSON_OBJ) {
        httpParseAll(route, key, prop);
    }
}


/*
    esp: {
        apps: 'myapp/esp.json',
        apps: [
            'apps/STAR/esp.json'
        ],
        apps: [
            { prefix: '/blog', config: 'blog/esp.json' }
        ],
    }
 */
static void parseApps(HttpRoute *route, cchar *key, MprJson *prop)
{
    MprJson     *child;
    int         ji;

    if (prop->type & MPR_JSON_STRING) {
        loadApp(route, prop);

    } else if (prop->type & MPR_JSON_OBJ) {
        loadApp(route, prop);

    } else if (prop->type & MPR_JSON_ARRAY) {
        for (ITERATE_CONFIG(route, prop, child, ji)) {
            loadApp(route, child);
        }
    }
}


static void parseCombine(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    if (smatch(prop->value, "true")) {
        eroute->combine = 1;
    } else {
        eroute->combine = 0;
    }
}


static void parseCompile(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    eroute->compile = (prop->type & MPR_JSON_TRUE) ? 1 : 0;
}


#if ME_WIN_LIKE
PUBLIC cchar *espGetVisualStudio()
{
    cchar   *path;
    int     v;

    if ((path = getenv("VSINSTALLDIR")) != 0) {
        return path;
    }
    /* VS 2015 == 14.0 */
    for (v = 16; v >= 8; v--) {
        if ((path = mprReadRegistry(ESP_VSKEY, sfmt("%d.0", v))) != 0) {
            path = strim(path, "\\", MPR_TRIM_END);
            break;
        }
    }
    if (!path) {
        path = "${VS}";
    }
    return path;
}


/*
    WARNING: yields
 */
PUBLIC int getVisualStudioEnv(Esp *esp)
{
    char    *error, *output, *next, *line, *key, *value;
    cchar   *arch, *cpu, *command, *vs;
    bool    yielding;

    /*
        Get the real system architecture, not whether this app is 32 or 64 bit.
        On native 64 bit systems, PA is amd64 for 64 bit apps and is PAW6432 is amd64 for 32 bit apps
     */
    if (smatch(getenv("PROCESSOR_ARCHITECTURE"), "AMD64") || getenv("PROCESSOR_ARCHITEW6432")) {
        cpu = "x64";
    } else {
        cpu = "x86";
    }
    httpParsePlatform(HTTP->platform, NULL, &arch, NULL);
    if (smatch(arch, "x64")) {
        arch = smatch(cpu, "x86") ? "x86_amd64" : "amd64";

    } else if (smatch(arch, "x86")) {
        arch = smatch(cpu, "x64") ? "amd64_x86" : "x86";

    } else if (smatch(arch, "arm")) {
        arch = smatch(cpu, "x86") ? "x86_arm" : "amd64_arm";

    } else {
        mprLog("error esp", 0, "Unsupported architecture %s", arch);
        return MPR_ERR_CANT_FIND;
    }
    vs = espGetVisualStudio();
    command = sfmt("\"%s\\vcvars.bat\" \"%s\" %s", mprGetAppDir(), mprJoinPath(vs, "VC/vcvarsall.bat"), arch);
    yielding = mprSetThreadYield(NULL, 0);
    if (mprRun(NULL, command, 0, &output, &error, -1) < 0) {
        mprLog("error esp", 0, "Cannot run command: %s, error %s", command, error);
        mprSetThreadYield(NULL, yielding);
        return MPR_ERR_CANT_READ;
    }
    mprSetThreadYield(NULL, yielding);
    esp->vstudioEnv = mprCreateHash(0, 0);

    next = output;
    while ((line = stok(next, "\r\n", &next)) != 0) {
        key = stok(line, "=", &value);
        if (scaselessmatch(key, "LIB") ||
            scaselessmatch(key, "INCLUDE") ||
            scaselessmatch(key, "PATH") ||
            scaselessmatch(key, "VSINSTALLDIR") ||
            scaselessmatch(key, "WindowsSdkDir") ||
            scaselessmatch(key, "WindowsSdkLibVersion")) {
            mprAddKey(esp->vstudioEnv, key, sclone(value));
        }
    }
    return 0;
}
#endif


static void defineEnv(HttpRoute *route, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    MprJson     *child, *set;
    cchar       *arch;
    int         ji;

    eroute = route->eroute;
    if (smatch(key, "set")) {
        httpParsePlatform(HTTP->platform, NULL, &arch, NULL);
#if ME_WIN_LIKE
        if (smatch(value, "VisualStudio")) {
            Esp     *esp;
            MprKey  *kp;

            /*
                Already set in users environment
             */
            if (scontains(getenv("LIB"), "Microsoft Visual Studio") &&
                scontains(getenv("INCLUDE"), "Microsoft Visual Studio") &&
                scontains(getenv("PATH"), "Microsoft Visual Studio")) {
                return;
            }
            /*
                By default, we use vsinstallvars.bat. However users can override by defining their own.
                WARNING: yields
             */
            esp = MPR->espService;
            if (!esp->vstudioEnv && getVisualStudioEnv(esp) < 0) {
                return;
            }
            for (ITERATE_KEYS(esp->vstudioEnv, kp)) {
                mprLog("info esp", 6, "define env %s %s", kp->key, kp->data);
                defineEnv(route, kp->key, kp->data);
            }
        }
        if (scontains(HTTP->platform, "-x64-") &&
            !(smatch(getenv("PROCESSOR_ARCHITECTURE"), "AMD64") || getenv("PROCESSOR_ARCHITEW6432"))) {
            /* Cross 64 */
            arch = sjoin(arch, "-cross", NULL);
        }
#endif
        if ((set = mprGetJsonObj(route->config, sfmt("esp.build.env.%s.default", value))) != 0) {
            for (ITERATE_CONFIG(route, set, child, ji)) {
                defineEnv(route, child->name, child->value);
            }
        }
        if ((set = mprGetJsonObj(route->config, sfmt("esp.build.env.%s.%s", value, arch))) != 0) {
            for (ITERATE_CONFIG(route, set, child, ji)) {
                defineEnv(route, child->name, child->value);
            }
        }

    } else {
        value = espExpandCommand(route, value, "", "");
        mprAddKey(eroute->env, key, value);
        if (scaselessmatch(key, "PATH")) {
            if (eroute->searchPath) {
                eroute->searchPath = sclone(value);
            } else {
                eroute->searchPath = sjoin(eroute->searchPath, MPR_SEARCH_SEP, value, NULL);
            }
        }
    }
}


static void parseBuild(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;
    MprJson     *child, *env, *rules;
    cchar       *buildType, *os, *rule, *stem;
    int         ji;

    eroute = route->eroute;
    buildType = HTTP->staticLink ? "static" : "dynamic";
    httpParsePlatform(HTTP->platform, &os, NULL, NULL);

    stem = sfmt("esp.build.rules.%s.%s", buildType, os);
    if ((rules = mprGetJsonObj(route->config, stem)) == 0) {
        stem = sfmt("esp.build.rules.%s.default", buildType);
        rules = mprGetJsonObj(route->config, stem);
    }
    if (rules) {
        if ((rule = mprGetJson(route->config, sfmt("%s.%s", stem, "compile"))) != 0) {
            eroute->compileCmd = rule;
        }
        if ((rule = mprGetJson(route->config, sfmt("%s.%s", stem, "link"))) != 0) {
            eroute->linkCmd = rule;
        }
        if ((env = mprGetJsonObj(route->config, sfmt("%s.%s", stem, "env"))) != 0) {
            if (eroute->env == 0) {
                eroute->env = mprCreateHash(-1, MPR_HASH_STABLE);
            }
            for (ITERATE_CONFIG(route, env, child, ji)) {
                /* WARNING: yields */
                defineEnv(route, child->name, child->value);
            }
        }
    } else {
        httpParseError(route, "Cannot find compile rules for O/S \"%s\"", os);
    }
}


static void parseKeep(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    eroute->keep = (prop->type & MPR_JSON_TRUE) ? 1 : 0;
}


static void parseOptimize(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    eroute->compileMode = smatch(prop->value, "true") ? ESP_COMPILE_OPTIMIZED : ESP_COMPILE_SYMBOLS;
}


static void parseUpdate(HttpRoute *route, cchar *key, MprJson *prop)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    eroute->update = (prop->type & MPR_JSON_TRUE) ? 1 : 0;
}


static void serverRouteSet(HttpRoute *route, cchar *set)
{
    httpAddRestfulRoute(route, "GET,POST", "/{action}(/)*$", "${action}", "{controller}");
}


static void restfulRouteSet(HttpRoute *route, cchar *set)
{
    httpAddResourceGroup(route, "{controller}");
}


static void postRouteSet(HttpRoute *route, cchar *set)
{
    httpAddPostGroup(route, "{controller}");
}

static void spaRouteSet(HttpRoute *route, cchar *set)
{
    httpAddSpaGroup(route, "{controller}");
}

#if DEPRECATED && REMOVE
static void legacyRouteSet(HttpRoute *route, cchar *set)
{
    restfulRouteSet(route, "restful");
}
#endif


PUBLIC int espInitParser()
{
    httpDefineRouteSet("esp-server", serverRouteSet);
    httpDefineRouteSet("esp-restful", restfulRouteSet);
    httpDefineRouteSet("esp-spa", spaRouteSet);
    httpDefineRouteSet("esp-post", postRouteSet);
#if DEPRECATE && REMOVE
    httpDefineRouteSet("esp-html-mvc", legacyRouteSet);
#endif
    httpAddConfig("esp", parseEsp);
    httpAddConfig("esp.app", parseApp);
    httpAddConfig("esp.apps", parseApps);
    httpAddConfig("esp.build", parseBuild);
    httpAddConfig("esp.combine", parseCombine);
    httpAddConfig("esp.compile", parseCompile);
    httpAddConfig("esp.keep", parseKeep);
    httpAddConfig("esp.optimize", parseOptimize);
    httpAddConfig("esp.update", parseUpdate);
    return 0;
}

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    espFramework.c -- ESP Web Framework API

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/



/************************************* Locals *********************************/

#define ITERATE_CONFIG(route, obj, child, index) \
    index = 0, child = obj ? obj->children: 0; obj && index < obj->length && !route->error; child = child->next, index++

/*********************************** Fowards **********************************/

static EspAction *createAction(cchar *target, cchar *abilities, void *callback);

/************************************* Code ***********************************/

#if DEPRECATED
PUBLIC void espAddPak(HttpRoute *route, cchar *name, cchar *version)
{
    if (!version || !*version || smatch(version, "0.0.0")) {
        version = "*";
    }
    mprSetJson(route->config, sfmt("dependencies.%s", name), version, MPR_JSON_STRING);
}
#endif


/*
    Add a http header if not already defined
 */
PUBLIC void espAddHeader(HttpStream *stream, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAddHeaderString(stream, key, sfmt(fmt, vargs));
    va_end(vargs);
}


/*
    Add a header string if not already defined
 */
PUBLIC void espAddHeaderString(HttpStream *stream, cchar *key, cchar *value)
{
    httpAddHeaderString(stream, key, value);
}


PUBLIC void espAddParam(HttpStream *stream, cchar *var, cchar *value)
{
    if (!httpGetParam(stream, var, 0)) {
        httpSetParam(stream, var, value);
    }
}


/*
   Append a header. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
PUBLIC void espAppendHeader(HttpStream *stream, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAppendHeaderString(stream, key, sfmt(fmt, vargs));
    va_end(vargs);
}


/*
   Append a header string. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
PUBLIC void espAppendHeaderString(HttpStream *stream, cchar *key, cchar *value)
{
    httpAppendHeaderString(stream, key, value);
}


PUBLIC void espAutoFinalize(HttpStream *stream)
{
    EspReq  *req;

    req = stream->reqData;
    if (req->autoFinalize) {
        httpFinalize(stream);
    }
}


PUBLIC int espCache(HttpRoute *route, cchar *uri, int lifesecs, int flags)
{
#if ME_HTTP_CACHE
    httpAddCache(route, NULL, uri, NULL, NULL, 0, lifesecs * TPS, flags);
#endif
    return 0;
}


PUBLIC cchar *espCreateSession(HttpStream *stream)
{
    HttpSession *session;

    if ((session = httpCreateSession(getStream())) != 0) {
        return session->id;
    }
    return 0;
}


#if DEPRECATED || 1
PUBLIC void espDefineAction(HttpRoute *route, cchar *target, EspProc callback)
{
    espAction(route, target, NULL, callback);
}
#endif


PUBLIC void espAction(HttpRoute *route, cchar *target, cchar *abilities, EspProc callback)
{
    EspRoute    *eroute;
    EspAction   *action;

    assert(route);
    assert(target && *target);
    assert(callback);

    eroute = ((EspRoute*) route->eroute)->top;
    if (target) {
#if DEPRECATED
        /* Keep till version 6 */
        if (scontains(target, "-cmd-")) {
            target = sreplace(target, "-cmd-", "/");
        } else if (schr(target, '-')) {
            controller = ssplit(sclone(target), "-", (char**) &action);
            target = sjoin(controller, "/", action, NULL);
        }
#endif
        if (!eroute->actions) {
            eroute->actions = mprCreateHash(-1, 0);
        }
        if ((action = createAction(target, abilities, callback)) == 0) {
            /* Memory errors centrally reported */
            return;
        }
        mprAddKey(eroute->actions, target, action);
    }
}


static void manageAction(EspAction *action, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(action->target);
        mprMark(action->abilities);
    }
}


static EspAction *createAction(cchar *target, cchar *abilities, void *callback)
{
    EspAction   *action;

    if ((action = mprAllocObj(EspAction, manageAction)) == 0) {
        return NULL;
    }
    action->target = sclone(target);
    action->abilities = abilities ? sclone(abilities) : NULL;
    action->callback = callback;
    return action;
}


#if DEPRECATED || 1
/*
    The base procedure is invoked prior to calling any and all actions on this route
 */
PUBLIC void espDefineBase(HttpRoute *route, EspLegacyProc baseProc)
{
    HttpRoute   *rp;
    EspRoute    *eroute;
    int         next;

    for (ITERATE_ITEMS(route->host->routes, rp, next)) {
        if ((eroute = rp->eroute) != 0) {
            if (smatch(httpGetDir(rp, "CONTROLLERS"), httpGetDir(route, "CONTROLLERS"))) {
                eroute->commonController = (EspProc) baseProc;
            }
        }
    }
}
#endif


/*
    Define a common base controller to invoke prior to calling any and all actions on this route
 */
PUBLIC void espController(HttpRoute *route, EspProc baseProc)
{
    HttpRoute   *rp;
    EspRoute    *eroute;
    int         next;

    for (ITERATE_ITEMS(route->host->routes, rp, next)) {
        if ((eroute = rp->eroute) != 0) {
            if (smatch(httpGetDir(rp, "CONTROLLERS"), httpGetDir(route, "CONTROLLERS"))) {
                eroute->commonController = baseProc;
            }
        }
    }
}


/*
    Path should be a relative path from route->documents to the view file (relative-path.esp)
 */
PUBLIC void espDefineView(HttpRoute *route, cchar *path, void *view)
{
    EspRoute    *eroute;

    assert(path && *path);
    assert(view);

    if (route->eroute) {
        eroute = ((EspRoute*) route->eroute)->top;
    } else {
        eroute = espRoute(route, 1);
        eroute = eroute->top;
    }
    if (route) {
        path = mprGetPortablePath(path);
    }
    if (!eroute->views) {
        eroute->views = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    }
    mprAddKey(eroute->views, path, view);
}


PUBLIC void espDestroySession(HttpStream *stream)
{
    httpDestroySession(stream);
}


PUBLIC void espFinalize(HttpStream *stream)
{
    httpFinalize(stream);
}


PUBLIC void espFlush(HttpStream *stream)
{
    httpFlush(stream);
}


PUBLIC HttpAuth *espGetAuth(HttpStream *stream)
{
    return stream->rx->route->auth;
}


PUBLIC cchar *espGetConfig(HttpRoute *route, cchar *key, cchar *defaultValue)
{
    cchar       *value;

    if (sstarts(key, "app.")) {
        mprLog("warn esp", 0, "Using legacy \"app\" configuration property");
    }
    if ((value = mprGetJson(route->config, key)) != 0) {
        return value;
    }
    return defaultValue;
}


PUBLIC MprOff espGetContentLength(HttpStream *stream)
{
    return httpGetContentLength(stream);
}


PUBLIC cchar *espGetContentType(HttpStream *stream)
{
    return stream->rx->mimeType;
}


PUBLIC cchar *espGetCookie(HttpStream *stream, cchar *name)
{
    return httpGetCookie(stream, name);
}


PUBLIC cchar *espGetCookies(HttpStream *stream)
{
    return httpGetCookies(stream);
}


PUBLIC void *espGetData(HttpStream *stream)
{
    EspReq  *req;

    req = stream->reqData;
    return req->data;
}


PUBLIC Edi *espGetDatabase(HttpStream *stream)
{
    HttpRx      *rx;
    EspReq      *req;
    EspRoute    *eroute;
    Edi         *edi;

    rx = stream->rx;
    req = stream->reqData;
    edi = req ? req->edi : 0;
    if (edi == 0 && rx && rx->route) {
        if ((eroute = rx->route->eroute) != 0) {
            edi = eroute->edi;
        }
    }
    if (edi == 0) {
        httpError(stream, 0, "Cannot get database instance");
        return 0;
    }
    return edi;
}


PUBLIC cchar *espGetDocuments(HttpStream *stream)
{
    return stream->rx->route->documents;
}


PUBLIC EspRoute *espGetEspRoute(HttpStream *stream)
{
    return stream->rx->route->eroute;
}


PUBLIC cchar *espGetFeedback(HttpStream *stream, cchar *kind)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *msg;

    req = stream->reqData;
    if (kind == 0 || req == 0 || req->feedback == 0 || mprGetHashLength(req->feedback) == 0) {
        return 0;
    }
    for (kp = 0; (kp = mprGetNextKey(req->feedback, kp)) != 0; ) {
        msg = kp->data;
        //  DEPRECATE "all"
        if (smatch(kind, kp->key) || smatch(kind, "all") || smatch(kind, "*")) {
            return msg;
        }
    }
    return 0;
}


PUBLIC EdiGrid *espGetGrid(HttpStream *stream)
{
    return stream->grid;
}


PUBLIC cchar *espGetHeader(HttpStream *stream, cchar *key)
{
    return httpGetHeader(stream, key);
}


PUBLIC MprHash *espGetHeaderHash(HttpStream *stream)
{
    return httpGetHeaderHash(stream);
}


PUBLIC char *espGetHeaders(HttpStream *stream)
{
    return httpGetHeaders(stream);
}


PUBLIC int espGetIntParam(HttpStream *stream, cchar *var, int defaultValue)
{
    return httpGetIntParam(stream, var, defaultValue);
}


PUBLIC cchar *espGetMethod(HttpStream *stream)
{
    return stream->rx->method;
}


PUBLIC cchar *espGetParam(HttpStream *stream, cchar *var, cchar *defaultValue)
{
    return httpGetParam(stream, var, defaultValue);
}


PUBLIC MprJson *espGetParams(HttpStream *stream)
{
    return httpGetParams(stream);
}


PUBLIC cchar *espGetPath(HttpStream *stream)
{
    return stream->rx->pathInfo;
}


PUBLIC cchar *espGetQueryString(HttpStream *stream)
{
    return httpGetQueryString(stream);
}


PUBLIC cchar *espGetReferrer(HttpStream *stream)
{
    if (stream->rx->referrer) {
        return stream->rx->referrer;
    }
    return httpLink(stream, "~");
}


PUBLIC HttpRoute *espGetRoute(HttpStream *stream)
{
    return stream->rx->route;
}


PUBLIC Edi *espGetRouteDatabase(HttpRoute *route)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    if (eroute == 0 || eroute->edi == 0) {
        return 0;
    }
    return eroute->edi;
}


PUBLIC cchar *espGetRouteVar(HttpStream *stream, cchar *var)
{
    return httpGetRouteVar(stream->rx->route, var);
}


PUBLIC cchar *espGetSessionID(HttpStream *stream, int create)
{
    HttpSession *session;

    if ((session = httpGetSession(getStream(), create)) != 0) {
        return session->id;
    }
    return 0;
}


PUBLIC int espGetStatus(HttpStream *stream)
{
    return httpGetStatus(stream);
}


PUBLIC cchar *espGetStatusMessage(HttpStream *stream)
{
    return httpGetStatusMessage(stream);
}


PUBLIC MprList *espGetUploads(HttpStream *stream)
{
    return stream->rx->files;
}


PUBLIC cchar *espGetUri(HttpStream *stream)
{
    return stream->rx->uri;
}


#if DEPRECATED

PUBLIC bool espHasPak(HttpRoute *route, cchar *name)
{
    return mprGetJsonObj(route->config, sfmt("dependencies.%s", name)) != 0;
}
#endif


PUBLIC bool espHasGrid(HttpStream *stream)
{
    return stream->grid != 0;
}


PUBLIC bool espHasRec(HttpStream *stream)
{
    EdiRec  *rec;

    rec = stream->record;
    return (rec && rec->id) ? 1 : 0;
}


PUBLIC bool espIsAuthenticated(HttpStream *stream)
{
    return httpIsAuthenticated(stream);
}


PUBLIC bool espIsEof(HttpStream *stream)
{
    return httpIsEof(stream);
}


PUBLIC bool espIsFinalized(HttpStream *stream)
{
    return httpIsFinalized(stream);
}


PUBLIC bool espIsSecure(HttpStream *stream)
{
    return stream->secure;
}


PUBLIC bool espMatchParam(HttpStream *stream, cchar *var, cchar *value)
{
    return httpMatchParam(stream, var, value);
}


/*
    Read rx data in non-blocking mode. Use standard connection timeouts.
 */
PUBLIC ssize espReceive(HttpStream *stream, char *buf, ssize len)
{
    return httpRead(stream, buf, len);
}


PUBLIC void espRedirect(HttpStream *stream, int status, cchar *target)
{
    httpRedirect(stream, status, target);
}


PUBLIC void espRedirectBack(HttpStream *stream)
{
    if (stream->rx->referrer) {
        espRedirect(stream, HTTP_CODE_MOVED_TEMPORARILY, stream->rx->referrer);
    }
}


PUBLIC ssize espRender(HttpStream *stream, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;

    va_start(vargs, fmt);
    buf = sfmtv(fmt, vargs);
    va_end(vargs);
    return espRenderString(stream, buf);
}


PUBLIC ssize espRenderBlock(HttpStream *stream, cchar *buf, ssize size)
{
    /*
        Must not yield as render() has dynamic allocations.
        If callers is generating a lot of data, they must call mprYield themselves or monitor the stream->writeq->count.
     */
    return httpWriteBlock(stream->writeq, buf, size, HTTP_BUFFER);
}


PUBLIC ssize espRenderCached(HttpStream *stream)
{
    return httpWriteCached(stream);
}


static void copyMappings(HttpRoute *route, MprJson *dest, MprJson *obj)
{
    MprJson     *child, *job, *jvalue;
    cchar       *key, *value;
    int         ji;

    for (ITERATE_CONFIG(route, obj, child, ji)) {
        if (child->type & MPR_JSON_OBJ) {
            job = mprCreateJson(MPR_JSON_OBJ);
            copyMappings(route, job, child);
            mprSetJsonObj(dest, child->name, job);
        } else {
            key = child->value;
            if (sends(key, "|time")) {
                key = ssplit(sclone(key), " \t|", NULL);
                if ((value = mprGetJson(route->config, key)) != 0) {
                    mprSetJson(dest, child->name, itos(httpGetTicks(value)), MPR_JSON_NUMBER);
                }
            } else {
                if ((jvalue = mprGetJsonObj(route->config, key)) != 0) {
                    mprSetJsonObj(dest, child->name, mprCloneJson(jvalue));
                }
            }
        }
    }
}


static cchar *getClientConfig(HttpStream *stream)
{
    HttpRoute   *route;
    MprJson     *mappings, *obj;

    stream = getStream();
    for (route = stream->rx->route; route; route = route->parent) {
        if (route->clientConfig) {
            return route->clientConfig;
        }
    }
    route = stream->rx->route;
    if ((obj = mprGetJsonObj(route->config, "esp.mappings")) != 0) {
        mappings = mprCreateJson(MPR_JSON_OBJ);
        copyMappings(route, mappings, obj);
        mprWriteJson(mappings, "prefix", route->prefix, 0);
        route->clientConfig = mprJsonToString(mappings, MPR_JSON_QUOTES);
    }
    return route->clientConfig;
}


PUBLIC ssize espRenderConfig(HttpStream *stream)
{
    cchar       *config;

    if ((config = getClientConfig(stream)) != 0) {
        return renderString(config);
    }
    return 0;
}


PUBLIC ssize espRenderError(HttpStream *stream, int status, cchar *fmt, ...)
{
    va_list     args;
    HttpRx      *rx;
    ssize       written;
    cchar       *msg, *title, *text;

    va_start(args, fmt);

    rx = stream->rx;
    if (rx->route->json) {
        mprLog("warn esp", 0, "Calling espRenderFeedback in JSON app");
        return 0 ;
    }
    written = 0;

    if (!httpIsFinalized(stream)) {
        if (status == 0) {
            status = HTTP_CODE_INTERNAL_SERVER_ERROR;
        }
        title = sfmt("Request Error for \"%s\"", rx->pathInfo);
        msg = mprEscapeHtml(sfmtv(fmt, args));
        if (rx->route->flags & HTTP_ROUTE_SHOW_ERRORS) {
            text = sfmt(\
                "<!DOCTYPE html>\r\n<html>\r\n<head><title>%s</title></head>\r\n" \
                "<body>\r\n<h1>%s</h1>\r\n" \
                "    <pre>%s</pre>\r\n" \
                "    <p>To prevent errors being displayed in the browser, " \
                "       set <b>http.showErrors off</b> in the JSON configuration file.</p>\r\n" \
                "</body>\r\n</html>\r\n", title, title, msg);
            httpSetContentType(stream, "text/html");
            written += espRenderString(stream, text);
            espFinalize(stream);
            httpLog(stream->trace, "esp.error", "error", "msg=%s, status=%d, uri=%s", msg, status, rx->pathInfo);
        }
    }
    va_end(args);
    return written;
}


PUBLIC ssize espRenderFile(HttpStream *stream, cchar *path)
{
    MprFile     *from;
    ssize       count, written, nbytes;
    char        buf[ME_BUFSIZE];

    if ((from = mprOpenFile(path, O_RDONLY | O_BINARY, 0)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    written = 0;
    while ((count = mprReadFile(from, buf, sizeof(buf))) > 0) {
        if ((nbytes = espRenderBlock(stream, buf, count)) < 0) {
            return nbytes;
        }
        written += nbytes;
    }
    mprCloseFile(from);
    return written;
}


PUBLIC ssize espRenderFeedback(HttpStream *stream, cchar *kinds)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *msg;
    ssize       written;

    req = stream->reqData;
    if (req->route->json) {
        mprLog("warn esp", 0, "Calling espRenderFeedback in JSON app");
        return 0;
    }
    if (kinds == 0 || req->feedback == 0 || mprGetHashLength(req->feedback) == 0) {
        return 0;
    }
    written = 0;
    for (kp = 0; (kp = mprGetNextKey(req->feedback, kp)) != 0; ) {
        msg = kp->data;
        //  DEPRECATE "all"
        if (strstr(kinds, kp->key) || strstr(kinds, "all") || strstr(kinds, "*")) {
            written += espRender(stream, "<span class='feedback-%s animate'>%s</span>", kp->key, msg);
        }
    }
    return written;
}


PUBLIC ssize espRenderSafe(HttpStream *stream, cchar *fmt, ...)
{
    va_list     args;
    cchar       *s;

    va_start(args, fmt);
    s = mprEscapeHtml(sfmtv(fmt, args));
    va_end(args);
    return espRenderBlock(stream, s, slen(s));
}


PUBLIC ssize espRenderSafeString(HttpStream *stream, cchar *s)
{
    s = mprEscapeHtml(s);
    return espRenderBlock(stream, s, slen(s));
}


PUBLIC ssize espRenderString(HttpStream *stream, cchar *s)
{
    return espRenderBlock(stream, s, slen(s));
}


/*
    Render a request variable. If a param by the given name is not found, consult the session.
 */
PUBLIC ssize espRenderVar(HttpStream *stream, cchar *name)
{
    cchar   *value;

    if ((value = espGetParam(stream, name, 0)) == 0) {
        value = httpGetSessionVar(stream, name, "");
    }
    return espRenderSafeString(stream, value);
}


PUBLIC int espRemoveHeader(HttpStream *stream, cchar *key)
{
    assert(key && *key);
    if (stream->tx == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return mprRemoveKey(stream->tx->headers, key);
}


PUBLIC void espRemoveSessionVar(HttpStream *stream, cchar *var)
{
    httpRemoveSessionVar(stream, var);
}


PUBLIC void espRemoveCookie(HttpStream *stream, cchar *name)
{
    HttpRoute   *route;
    cchar       *url;

    route = stream->rx->route;
    url = (route->prefix && *route->prefix) ? route->prefix : "/";
    httpSetCookie(stream, name, "", url, NULL, 0, 0);
}


PUBLIC void espSetStream(HttpStream *stream)
{
    mprSetThreadData(((Esp*) MPR->espService)->local, stream);
}


static void espNotifier(HttpStream *stream, int event, int arg)
{
    EspReq      *req;

    if ((req = stream->reqData) != 0) {
        espSetStream(stream);
        (req->notifier)(stream, event, arg);
    }
}


PUBLIC void espSetNotifier(HttpStream *stream, HttpNotifier notifier)
{
    EspReq      *req;

    if ((req = stream->reqData) != 0) {
        req->notifier = notifier;
        httpSetStreamNotifier(stream, espNotifier);
    }
}


#if DEPRECATED
PUBLIC int espSaveConfig(HttpRoute *route)
{
    cchar       *path;

    path = mprJoinPath(route->home, "esp.json");
#if KEEP
    mprBackupLog(path, 3);
#endif
    return mprSaveJson(route->config, path, MPR_JSON_PRETTY | MPR_JSON_QUOTES);
}
#endif


/*
    Send a grid with schema
 */
PUBLIC ssize espSendGrid(HttpStream *stream, EdiGrid *grid, int flags)
{
    HttpRoute   *route;
    EspRoute    *eroute;

    route = stream->rx->route;

    if (route->json) {
        httpSetContentType(stream, "application/json");
        if (grid) {
            eroute = route->eroute;
            flags = flags | (eroute->encodeTypes ? MPR_JSON_ENCODE_TYPES : 0);
            return espRender(stream, "{\n  \"data\": %s, \"count\": %d, \"schema\": %s}\n",
                ediGridAsJson(grid, flags), grid->count, ediGetGridSchemaAsJson(grid));
        }
        return espRender(stream, "{data:[]}");
    }
    return 0;
}


PUBLIC ssize espSendRec(HttpStream *stream, EdiRec *rec, int flags)
{
    HttpRoute   *route;
    EspRoute    *eroute;

    route = stream->rx->route;
    if (route->json) {
        httpSetContentType(stream, "application/json");
        if (rec) {
            eroute = route->eroute;
            flags = flags | (eroute->encodeTypes ? MPR_JSON_ENCODE_TYPES : 0);
            return espRender(stream, "{\n  \"data\": %s, \"schema\": %s}\n", ediRecAsJson(rec, flags), ediGetRecSchemaAsJson(rec));
        }
        return espRender(stream, "{}");
    }
    return 0;
}


PUBLIC ssize espSendResult(HttpStream *stream, bool success)
{
    EspReq      *req;
    EdiRec      *rec;
    ssize       written;

    req = stream->reqData;
    written = 0;
    if (req->route->json) {
        rec = getRec();
        if (rec && rec->errors) {
            written = espRender(stream, "{\"error\": %d, \"feedback\": %s, \"fieldErrors\": %s}", !success,
                req->feedback ? mprSerialize(req->feedback, MPR_JSON_QUOTES) : "{}",
                mprSerialize(rec->errors, MPR_JSON_QUOTES));
        } else {
            written = espRender(stream, "{\"error\": %d, \"feedback\": %s}", !success,
                req->feedback ? mprSerialize(req->feedback, MPR_JSON_QUOTES) : "{}");
        }
        espFinalize(stream);
    } else {
        /* Noop */
    }
    return written;
}


PUBLIC bool espSetAutoFinalizing(HttpStream *stream, bool on)
{
    EspReq  *req;
    bool    old;

    req = stream->reqData;
    old = req->autoFinalize;
    req->autoFinalize = on;
    return old;
}


PUBLIC int espSetConfig(HttpRoute *route, cchar *key, cchar *value)
{
    return mprSetJson(route->config, key, value, 0);
}


PUBLIC void espSetContentLength(HttpStream *stream, MprOff length)
{
    httpSetContentLength(stream, length);
}


PUBLIC void espSetCookie(HttpStream *stream, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, MprTicks lifespan,
        bool isSecure)
{
    httpSetCookie(stream, name, value, path, cookieDomain, lifespan, isSecure);
}


PUBLIC void espSetContentType(HttpStream *stream, cchar *mimeType)
{
    httpSetContentType(stream, mimeType);
}


PUBLIC void espSetData(HttpStream *stream, void *data)
{
    EspReq  *req;

    req = stream->reqData;
    req->data = data;
}


PUBLIC void espSetFeedback(HttpStream *stream, cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFeedbackv(stream, kind, fmt, args);
    va_end(args);
}


PUBLIC void espSetFeedbackv(HttpStream *stream, cchar *kind, cchar *fmt, va_list args)
{
    EspReq      *req;
    cchar       *msg;

    if ((req = stream->reqData) == 0) {
        return;
    }
    if (!req->route->json) {
        /*
            Create a session as early as possible so a Set-Cookie header can be omitted.
         */
        httpGetSession(stream, 1);
    }
    if (req->feedback == 0) {
        req->feedback = mprCreateHash(0, MPR_HASH_STABLE);
    }
    msg = sfmtv(fmt, args);

#if KEEP
    MprKey      *current, *last;
    if ((current = mprLookupKeyEntry(req->feedback, kind)) != 0) {
        if ((last = mprLookupKey(req->lastFeedback, current->key)) != 0 && current->data == last->data) {
            /* Overwrite prior feedback messages */
            mprAddKey(req->feedback, kind, msg);
        } else {
            /* Append to existing feedback messages */
            mprAddKey(req->feedback, kind, sjoin(current->data, ", ", msg, NULL));
        }
    } else
#endif
    mprAddKey(req->feedback, kind, msg);
}


#if DEPRECATED
PUBLIC void espSetFlash(HttpStream *stream, cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFeedbackv(stream, kind, fmt, args);
    va_end(args);
}
#endif


PUBLIC EdiGrid *espSetGrid(HttpStream *stream, EdiGrid *grid)
{
    return stream->grid = grid;
}


/*
    Set a http header. Overwrite if present.
 */
PUBLIC void espSetHeader(HttpStream *stream, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    httpSetHeaderString(stream, key, sfmtv(fmt, vargs));
    va_end(vargs);
}


PUBLIC void espSetHeaderString(HttpStream *stream, cchar *key, cchar *value)
{
    httpSetHeaderString(stream, key, value);
}


PUBLIC void espSetIntParam(HttpStream *stream, cchar *var, int value)
{
    httpSetIntParam(stream, var, value);
}


PUBLIC void espSetParam(HttpStream *stream, cchar *var, cchar *value)
{
    httpSetParam(stream, var, value);
}


PUBLIC EdiRec *espSetRec(HttpStream *stream, EdiRec *rec)
{
    return stream->record = rec;
}


PUBLIC int espSetSessionVar(HttpStream *stream, cchar *var, cchar *value)
{
    return httpSetSessionVar(stream, var, value);
}


PUBLIC void espSetStatus(HttpStream *stream, int status)
{
    httpSetStatus(stream, status);
}


PUBLIC void espShowRequest(HttpStream *stream)
{
    MprHash     *env;
    MprJson     *params, *param;
    MprKey      *kp;
    MprJson     *jkey;
    HttpRx      *rx;
    int         i;

    rx = stream->rx;
    httpAddHeaderString(stream, "Cache-Control", "no-cache");
    httpCreateCGIParams(stream);
    espRender(stream, "\r\n");

    /*
        Query
     */
    for (ITERATE_JSON(rx->params, jkey, i)) {
        espRender(stream, "PARAMS %s=%s\r\n", jkey->name, jkey->value ? jkey->value : "null");
    }
    espRender(stream, "\r\n");

    /*
        Http Headers
     */
    env = espGetHeaderHash(stream);
    for (ITERATE_KEYS(env, kp)) {
        espRender(stream, "HEADER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espRender(stream, "\r\n");

    /*
        Server vars
     */
    for (ITERATE_KEYS(stream->rx->svars, kp)) {
        espRender(stream, "SERVER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espRender(stream, "\r\n");

    /*
        Form vars
     */
    if ((params = espGetParams(stream)) != 0) {
        for (ITERATE_JSON(params, param, i)) {
            espRender(stream, "FORM %s=%s\r\n", param->name, param->value);
        }
        espRender(stream, "\r\n");
    }

#if KEEP
    /*
        Body
     */
    q = stream->readq;
    if (q->first && rx->bytesRead > 0 && scmp(rx->mimeType, "application/x-www-form-urlencoded") == 0) {
        buf = q->first->content;
        mprAddNullToBuf(buf);
        if ((numKeys = getParams(&keys, mprGetBufStart(buf), (int) mprGetBufLength(buf))) > 0) {
            for (i = 0; i < (numKeys * 2); i += 2) {
                value = keys[i+1];
                espRender(stream, "BODY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espRender(stream, "\r\n");
    }
#endif
}


PUBLIC bool espTestConfig(HttpRoute *route, cchar *key, cchar *desired)
{
    cchar       *value;

    if ((value = mprGetJson(route->config, key)) != 0) {
        return smatch(value, desired);
    }
    return 0;
}


PUBLIC void espUpdateCache(HttpStream *stream, cchar *uri, cchar *data, int lifesecs)
{
    httpUpdateCache(stream, uri, data, lifesecs * TPS);
}


PUBLIC cchar *espUri(HttpStream *stream, cchar *target)
{
    return httpLink(stream, target);
}


PUBLIC int espEmail(HttpStream *stream, cchar *to, cchar *from, cchar *subject, MprTime date, cchar *mime,
    cchar *message, MprList *files)
{
    MprList         *lines;
    MprCmd          *cmd;
    cchar           *body, *boundary, *contents, *encoded, *file;
    char            *out, *err;
    ssize           length;
    int             i, next, status;

    if (!from || !*from) {
        from = "anonymous";
    }
    if (!subject || !*subject) {
        subject = "Mail message";
    }
    if (!mime || !*mime) {
        mime = "text/plain";
    }
    if (!date) {
        date = mprGetTime();
    }
    boundary = sjoin("esp.mail=", mprGetMD5("BOUNDARY"), NULL);
    lines = mprCreateList(0, 0);

    mprAddItem(lines, sfmt("To: %s", to));
    mprAddItem(lines, sfmt("From: %s", from));
    mprAddItem(lines, sfmt("Date: %s", mprFormatLocalTime(0, date)));
    mprAddItem(lines, sfmt("Subject: %s", subject));
    mprAddItem(lines, "MIME-Version: 1.0");
    mprAddItem(lines, sfmt("Content-Type: multipart/mixed; boundary=%s", boundary));
    mprAddItem(lines, "");

    boundary = sjoin("--", boundary, NULL);

    mprAddItem(lines, boundary);
    mprAddItem(lines, sfmt("Content-Type: %s", mime));
    mprAddItem(lines, "");
    mprAddItem(lines, "");
    mprAddItem(lines, message);

    for (ITERATE_ITEMS(files, file, next)) {
        mprAddItem(lines, boundary);
        if ((mime = mprLookupMime(NULL, file)) == 0) {
            mime = "application/octet-stream";
        }
        mprAddItem(lines, "Content-Transfer-Encoding: base64");
        mprAddItem(lines, sfmt("Content-Disposition: inline; filename=\"%s\"", mprGetPathBase(file)));
        mprAddItem(lines, sfmt("Content-Type: %s; name=\"%s\"", mime, mprGetPathBase(file)));
        mprAddItem(lines, "");
        contents = mprReadPathContents(file, &length);
        encoded = mprEncode64Block(contents, length);
        for (i = 0; i < length; i += 76) {
            mprAddItem(lines, snclone(&encoded[i], i + 76));
        }
    }
    mprAddItem(lines, sfmt("%s--", boundary));

    body = mprListToString(lines, "\n");
    httpLog(stream->trace, "esp.email", "context", "%s", body);

    cmd = mprCreateCmd(stream->dispatcher);
    if (mprRunCmd(cmd, "sendmail -t", NULL, body, &out, &err, -1, 0) < 0) {
        mprDestroyCmd(cmd);
        return MPR_ERR_CANT_OPEN;
    }
    if (mprWaitForCmd(cmd, ME_ESP_EMAIL_TIMEOUT) < 0) {
        httpLog(stream->trace, "esp.email.error", "error",
            "msg=\"Timeout waiting for command to complete\", timeout=%d, command=\"%s\"",
            ME_ESP_EMAIL_TIMEOUT, cmd->argv[0]);
        mprDestroyCmd(cmd);
        return MPR_ERR_CANT_COMPLETE;
    }
    if ((status = mprGetCmdExitStatus(cmd)) != 0) {
        httpLog(stream->trace, "esp.email.error", "error", "msg=\"Sendmail failed\", status=%d, error=\"%s\"", status, err);
        mprDestroyCmd(cmd);
        return MPR_ERR_CANT_WRITE;
    }
    mprDestroyCmd(cmd);
    return 0;
}


PUBLIC void espClearCurrentSession(HttpStream *stream)
{
    EspRoute    *eroute;

    eroute = stream->rx->route->eroute;
    if (eroute->currentSession) {
        httpLog(stream->trace, "esp.singular.clear", "context", "session=%s", eroute->currentSession);
    }
    eroute->currentSession = 0;
}


/*
    Remember this connections session as the current session. Use for single login tracking.
 */
PUBLIC void espSetCurrentSession(HttpStream *stream)
{
    EspRoute    *eroute;

    eroute = stream->rx->route->eroute;
    eroute->currentSession = httpGetSessionID(stream);
    httpLog(stream->trace, "esp.singular.set", "context", "msg=\"Set singluar user\", session=%s", eroute->currentSession);
}


/*
    Test if this connection is the current session. Use for single login tracking.
 */
PUBLIC bool espIsCurrentSession(HttpStream *stream)
{
    EspRoute    *eroute;

    eroute = stream->rx->route->eroute;
    if (eroute->currentSession) {
        if (smatch(httpGetSessionID(stream), eroute->currentSession)) {
            return 1;
        }
        if (httpLookupSessionID(eroute->currentSession)) {
            /* Session is still current */
            return 0;
        }
        /* Session has expired */
        eroute->currentSession = 0;
    }
    return 1;
}


/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    espHtml.c -- ESP HTML controls 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/




/************************************* Local **********************************/

static cchar *getValue(HttpStream *stream, cchar *fieldName, MprHash *options);
static cchar *map(HttpStream *stream, MprHash *options);

/************************************* Code ***********************************/

PUBLIC void input(cchar *field, cchar *optionString)
{
    HttpStream    *stream;
    MprHash     *choices, *options;
    MprKey      *kp;
    EdiRec      *rec;
    cchar       *rows, *cols, *etype, *value, *checked, *style, *error, *errorMsg;
    int         type, flags;

    stream = getStream();
    rec = stream->record;
    if (ediGetColumnSchema(rec->edi, rec->tableName, field, &type, &flags, NULL) < 0) {
        type = -1;
    }
    options = httpGetOptions(optionString);
    style = httpGetOption(options, "class", "");
    errorMsg = rec->errors ? mprLookupKey(rec->errors, field) : 0;
    error = errorMsg ? sfmt("<span class=\"field-error\">%s</span>", errorMsg) : ""; 

    switch (type) {
    case EDI_TYPE_BOOL:
        choices = httpGetOptions("{off: 0, on: 1}");
        value = getValue(stream, field, options);
        for (kp = 0; (kp = mprGetNextKey(choices, kp)) != 0; ) {
            checked = (smatch(kp->data, value)) ? " checked" : "";
            espRender(stream, "%s <input type='radio' name='%s' value='%s'%s%s class='%s'/>\r\n",
                stitle(kp->key), field, kp->data, checked, map(stream, options), style);
        }
        break;
        /* Fall through */
    case EDI_TYPE_BINARY:
    default:
        httpError(stream, 0, "espInput: unknown field type %d", type);
        /* Fall through */
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_TEXT:

    case EDI_TYPE_INT:
    case EDI_TYPE_DATE:
    case EDI_TYPE_STRING:        
        if (type == EDI_TYPE_TEXT && !httpGetOption(options, "rows", 0)) {
            httpSetOption(options, "rows", "10");
        }
        etype = "text";
        value = getValue(stream, field, options);
        if (value == 0 || *value == '\0') {
            value = espGetParam(stream, field, "");
        }
        if (httpGetOption(options, "password", 0)) {
            etype = "password";
        } else if (httpGetOption(options, "hidden", 0)) {
            etype = "hidden";
        }
        if ((rows = httpGetOption(options, "rows", 0)) != 0) {
            cols = httpGetOption(options, "cols", "60");
            espRender(stream, "<textarea name='%s' type='%s' cols='%s' rows='%s'%s class='%s'>%s</textarea>", 
                field, etype, cols, rows, map(stream, options), style, value);
        } else {
            espRender(stream, "<input name='%s' type='%s' value='%s'%s class='%s'/>", field, etype, value, 
                map(stream, options), style);
        }
        if (error) {
            espRenderString(stream, error);
        }
        break;
    }
}


/*
    Render an input field with a hidden security token
    Used by esp-html-mvc to add XSRF tokens to a form
 */
PUBLIC void inputSecurityToken()
{
    HttpStream    *stream;

    stream = getStream();
    espRender(stream, "    <input name='%s' type='hidden' value='%s' />\r\n", ME_XSRF_PARAM, httpGetSecurityToken(stream, 0));
}


/**************************************** Support *************************************/ 

static cchar *getValue(HttpStream *stream, cchar *fieldName, MprHash *options)
{
    EdiRec      *record;
    cchar       *value;

    record = stream->record;
    value = 0;
    if (record) {
        value = ediGetFieldValue(record, fieldName);
    }
    if (value == 0) {
        value = httpGetOption(options, "value", 0);
    }
    if (!httpGetOption(options, "noescape", 0)) {
        value = mprEscapeHtml(value);
    }
    return value;
}


/*
    Map options to an attribute string.
 */
static cchar *map(HttpStream *stream, MprHash *options)
{
    MprKey      *kp;
    MprBuf      *buf;

    if (options == 0 || mprGetHashLength(options) == 0) {
        return MPR->emptyString;
    }
    buf = mprCreateBuf(-1, -1);
    for (kp = 0; (kp = mprGetNextKey(options, kp)) != 0; ) {
        if (kp->type != MPR_JSON_OBJ && kp->type != MPR_JSON_ARRAY) {
            mprPutCharToBuf(buf, ' ');
            mprPutStringToBuf(buf, kp->key);
            mprPutStringToBuf(buf, "='");
            mprPutStringToBuf(buf, kp->data);
            mprPutCharToBuf(buf, '\'');
        }
    }
    mprAddNullToBuf(buf);
    return mprGetBufStart(buf);
}

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    espRequest.c -- ESP Request handler

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/



/************************************* Local **********************************/
/*
    Singleton ESP control structure
 */
static Esp *esp;

/*
    espRenderView flags are reserved (UNUSED)
 */
#define ESP_DONT_RENDER 0x1

/************************************ Forward *********************************/

static int cloneDatabase(HttpStream *stream);
static void closeEsp(HttpQueue *q);
static void ifConfigModified(HttpRoute *route, cchar *path, bool *modified);
static void manageEsp(Esp *esp, int flags);
static void manageReq(EspReq *req, int flags);
static int openEsp(HttpQueue *q);
static int runAction(HttpStream *stream);
static void startEsp(HttpQueue *q);
static int unloadEsp(MprModule *mp);

#if !ME_STATIC
static cchar *getCacheName(HttpRoute *route, cchar *kind, cchar *source);
static int espLoadModule(HttpRoute *route, MprDispatcher *dispatcher, cchar *kind, cchar *source, cchar **errMsg, bool *loaded);
static cchar *getModuleName(HttpRoute *route, cchar *kind, cchar *target);
static char *getModuleEntry(EspRoute *eroute, cchar *kind, cchar *source, cchar *cache);
static bool layoutIsStale(EspRoute *eroute, cchar *source, cchar *module);
#endif

/************************************* Code ***********************************/
/*
    Load and initialize ESP module. Manually loaded when used inside esp.c.
 */
PUBLIC int espOpen(MprModule *module)
{
    HttpStage   *handler;

    if ((handler = httpCreateHandler("espHandler", module)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    HTTP->espHandler = handler;
    handler->open = openEsp;
    handler->close = closeEsp;
    handler->start = startEsp;
    handler->outgoingService = httpDefaultService;

    /*
        Using the standard 'incoming' callback that simply transfers input to the queue head
        Applications should read by defining a notifier for READABLE events and then calling httpGetPacket
        on the read queue.
     */
    if ((esp = mprAllocObj(Esp, manageEsp)) == 0) {
        return MPR_ERR_MEMORY;
    }
    MPR->espService = esp;
    handler->stageData = esp;
    esp->mutex = mprCreateLock();
    esp->local = mprCreateThreadLocal();

    if (espInitParser() < 0) {
        return 0;
    }
    if ((esp->ediService = ediCreateService()) == 0) {
        return 0;
    }
#if ME_COM_MDB
    mdbInit();
#endif
#if ME_COM_SQLITE
    sdbInit();
#endif
    if (module) {
        mprSetModuleFinalizer(module, unloadEsp);
    }
    return 0;
}


static int unloadEsp(MprModule *mp)
{
    HttpStage   *stage;

    if (esp->inUse) {
       return MPR_ERR_BUSY;
    }
    if (mprIsStopping()) {
        return 0;
    }
    if ((stage = httpLookupStage(mp->name)) != 0) {
        stage->flags |= HTTP_STAGE_UNLOADED;
    }
    return 0;
}


/*
    Open an instance of the ESP for a new request
 */
static int openEsp(HttpQueue *q)
{
    HttpStream  *stream;
    HttpRx      *rx;
    HttpRoute   *rp, *route;
    EspRoute    *eroute;
    EspReq      *req;
    cchar       *cookie;
    int         next;

    stream = q->stream;
    rx = stream->rx;
    route = rx->route;

    if ((req = mprAllocObj(EspReq, manageReq)) == 0) {
        httpMemoryError(stream);
        return MPR_ERR_MEMORY;
    }
    stream->reqData = req;

    /*
        If unloading a module, this lock will cause a wait here while ESP applications are reloaded.
     */
    lock(esp);
    esp->inUse++;
    unlock(esp);

    /*
        Find the ESP route configuration. Search up the route parent chain.
     */
    for (eroute = 0, rp = route; rp; rp = rp->parent) {
        if (rp->eroute) {
            eroute = rp->eroute;
            break;
        }
    }
    if (!eroute) {
        /* WARNING: may yield */
        if (espInit(route, 0, "esp.json") < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
        eroute = route->eroute;
    } else {
        route->eroute = eroute;
    }
    req->esp = esp;
    req->route = route;
    req->autoFinalize = route->autoFinalize;

    /*
        If a cookie is not explicitly set, use the application name for the session cookie so that
        cookies are unique per esp application.
     */
    cookie = route->cookie;
    if (!cookie) {
        cookie = sfmt("esp-%s", eroute->appName);
    }
    for (ITERATE_ITEMS(route->host->routes, rp, next)) {
        if (!rp->cookie) {
            httpSetRouteCookie(rp, cookie);
        }
    }
    return 0;
}


static void closeEsp(HttpQueue *q)
{
    lock(esp);
    esp->inUse--;
    assert(esp->inUse >= 0);
    unlock(esp);
}


#if !ME_STATIC
/*
    This is called when unloading a view or controller module
    All of ESP must be quiesced.
 */
static bool espUnloadModule(cchar *module, MprTicks timeout)
{
    MprModule   *mp;
    MprTicks    mark;

    if ((mp = mprLookupModule(module)) != 0) {
        mark = mprGetTicks();
        esp->reloading = 1;
        do {
            lock(esp);
            /* Own request will count as 1 */
            if (esp->inUse <= 1) {
                if (mprUnloadModule(mp) < 0) {
                    mprLog("error esp", 0, "Cannot unload module %s", mp->name);
                    unlock(esp);
                }
                esp->reloading = 0;
                unlock(esp);
                return 1;
            }
            unlock(esp);
            mprSleep(10);

        } while (mprGetRemainingTicks(mark, timeout) > 0);
        esp->reloading = 0;
    }
    return 0;
}
#endif


/*
    Not used
 */
PUBLIC void espClearFeedback(HttpStream *stream)
{
    EspReq      *req;

    req = stream->reqData;
    req->feedback = 0;
}


static void setupFeedback(HttpStream *stream)
{
    EspReq      *req;

    req = stream->reqData;
    req->lastFeedback = 0;
    if (req->route->json) {
        req->feedback = mprCreateHash(0, MPR_HASH_STABLE);
    } else {
        if (httpGetSession(stream, 0)) {
            req->feedback = httpGetSessionObj(stream, ESP_FEEDBACK_VAR);
            if (req->feedback) {
                httpRemoveSessionVar(stream, ESP_FEEDBACK_VAR);
                req->lastFeedback = mprCloneHash(req->feedback);
            }
        }
    }
}


static void finalizeFeedback(HttpStream *stream)
{
    EspReq  *req;
    MprKey  *kp, *lp;

    req = stream->reqData;
    if (req->feedback) {
        if (req->route->json) {
            if (req->lastFeedback) {
                for (ITERATE_KEYS(req->feedback, kp)) {
                    if ((lp = mprLookupKeyEntry(req->lastFeedback, kp->key)) != 0 && kp->data == lp->data) {
                        mprRemoveKey(req->feedback, kp->key);
                    }
                }
            }
            if (mprGetHashLength(req->feedback) > 0) {
                /*
                    If the session does not exist, this will create one. However, must not have
                    emitted the headers, otherwise cannot inform the client of the session cookie.
                */
                httpSetSessionObj(stream, ESP_FEEDBACK_VAR, req->feedback);
            }
        }
    }
}


/*
    Start the request. At this stage, body data may not have been fully received unless
    the request is a form (POST method and content type is application/x-www-form-urlencoded).
    Forms are a special case and delay invoking the start callback until all body data is received.
    WARNING: GC yield
 */
static void startEsp(HttpQueue *q)
{
    HttpStream  *stream;
    HttpRx      *rx;
    EspReq      *req;

    stream = q->stream;
    rx = stream->rx;
    req = stream->reqData;

#if ME_WIN_LIKE
    rx->target = mprGetPortablePath(rx->target);
#endif

    if (req) {
        mprSetThreadData(req->esp->local, stream);
        /* WARNING: GC yield */
        if (runAction(stream)) {
            if (!stream->error && req->autoFinalize) {
                if (!stream->tx->responded) {
                    /* WARNING: GC yield */
                    espRenderDocument(stream, rx->target);
                }
                if (req->autoFinalize) {
                    espFinalize(stream);
                }
            }
        }
        finalizeFeedback(stream);
        mprSetThreadData(req->esp->local, NULL);
    }
}


/*
    Yields
 */
static bool loadController(HttpStream *stream)
{
#if !ME_STATIC
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    cchar       *errMsg, *controllers, *controller;
    bool        loaded;

    rx = stream->rx;
    route = rx->route;
    eroute = route->eroute;
    if (!eroute->combine && (eroute->update || !mprLookupKey(eroute->actions, rx->target))) {
        if ((controllers = httpGetDir(route, "CONTROLLERS")) == 0) {
            controllers = ".";
        }
        controllers = mprJoinPath(route->home, controllers);
        controller = schr(route->sourceName, '$') ? stemplateJson(route->sourceName, rx->params) : route->sourceName;
        controller = controllers ? mprJoinPath(controllers, controller) : mprJoinPath(route->home, controller);

        /* May yield */
        route->source = controller;
        if (espLoadModule(route, stream->dispatcher, "controller", controller, &errMsg, &loaded) < 0) {
            if (mprPathExists(controller, R_OK)) {
                httpError(stream, HTTP_CODE_INTERNAL_SERVER_ERROR, "%s", errMsg);
                return 0;
#if UNUSED
            } else {
                //  Cant do this because esp pages try to load a controller first and then fall back to RenderDocument
                httpError(stream, HTTP_CODE_NOT_FOUND, "%s", errMsg);
#endif
            }
        } else if (loaded) {
            httpLog(stream->trace, "esp.handler", "context", "msg:Load module %s", controller);
        }
    }
#endif /* !ME_STATIC */
    return 1;
}


static bool setToken(HttpStream *stream)
{
    HttpRx      *rx;
    HttpRoute   *route;

    rx = stream->rx;
    route = rx->route;

    if (route->flags & HTTP_ROUTE_XSRF && !(rx->flags & HTTP_GET)) {
        if (!httpCheckSecurityToken(stream)) {
            httpSetStatus(stream, HTTP_CODE_UNAUTHORIZED);
            if (route->json) {
                httpLog(stream->trace, "esp.xsrf.error", "error", 0);
                espRenderString(stream,
                    "{\"retry\": true, \"success\": 0, \"feedback\": {\"error\": \"Security token is stale. Please retry.\"}}");
                espFinalize(stream);
            } else {
                httpError(stream, HTTP_CODE_UNAUTHORIZED, "Security token is stale. Please reload page.");
            }
            return 0;
        }
    }
    return 1;
}


/*
    Run an action (may yield)
 */
static int runAction(HttpStream *stream)
{
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    EspReq      *req;
    EspAction   *action;

    rx = stream->rx;
    req = stream->reqData;
    route = rx->route;
    eroute = route->eroute;
    assert(eroute);

    if (eroute->edi && eroute->edi->flags & EDI_PRIVATE) {
        cloneDatabase(stream);
    } else {
        req->edi = eroute->edi;
    }
    if (route->sourceName == 0 || *route->sourceName == '\0') {
        if (eroute->commonController) {
            (eroute->commonController)(stream, NULL);
        }
        return 1;
    }
    /* May yield */
    if (!loadController(stream)) {
        return 0;
    }
    if (!setToken(stream)) {
        return 0;
    }
    httpAuthenticate(stream);

    action = mprLookupKey(eroute->top->actions, rx->target);
    httpLog(stream->trace, "esp.handler", "context", "msg:Invoke controller action %s", rx->target);

    if (eroute->commonController) {
        (eroute->commonController)(stream, action);
    }
    if (httpIsFinalized(stream)) {
        return 1;
    }
    assert(eroute->top);
    if (action) {
        setupFeedback(stream);
        (action->callback)(stream, action);
    }
    return 1;
}


/*
    Load a view ... may yield
 */
static bool loadView(HttpStream *stream, cchar *target)
{
#if !ME_STATIC
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    bool        loaded;
    cchar       *errMsg, *path;

    rx = stream->rx;
    route = rx->route;
    eroute = route->eroute;
    assert(eroute);

    if (!eroute->combine && (eroute->update || !mprLookupKey(eroute->top->views, target))) {
        path = mprJoinPath(route->documents, target);
        httpLog(stream->trace, "esp.handler", "context", "msg:Loading module %s", path);
        /* May yield */
        route->source = path;
        if (espLoadModule(route, stream->dispatcher, "view", path, &errMsg, &loaded) < 0) {
            httpError(stream, HTTP_CODE_NOT_FOUND, "%s", errMsg);
            return 0;
        }
    }
#endif
    return 1;
}

/*
    WARNING: this can yield
 */
PUBLIC bool espRenderView(HttpStream *stream, cchar *target, int flags)
{
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    EspViewProc viewProc;

    rx = stream->rx;
    route = rx->route;
    eroute = route->eroute;

    /* WARNING: may yield */
    if (!loadView(stream, target)) {
        return 0;
    }
    if ((viewProc = mprLookupKey(eroute->top->views, target)) == 0) {
        httpError(stream, HTTP_CODE_NOT_FOUND, "Cannot find view %s", target);
        return 0;
    }
    if (!(flags & ESP_DONT_RENDER)) {
        if (route->flags & HTTP_ROUTE_XSRF) {
            /* Add a new unique security token */
            httpAddSecurityToken(stream, 1);
        }
        httpSetContentType(stream, "text/html");
        httpSetFilename(stream, mprJoinPath(route->documents, target), 0);
        /* WARNING: may yield */
        (viewProc)(stream);
    }
    return 1;
}


/*
    Check if the target/filename.ext is registered as an esp view
 */
static cchar *checkView(HttpStream *stream, cchar *target, cchar *filename, cchar *ext)
{
    HttpRx          *rx;
    HttpRoute       *route;
    EspRoute        *eroute;
    MprFileSystem   *fs;

    assert(target);

    rx = stream->rx;
    route = rx->route;
    eroute = route->eroute;

    if (filename) {
        target = mprJoinPath(target, filename);
    }
    assert(target && *target);

    if (ext && *ext) {
        fs = mprLookupFileSystem("/");
        if (fs->caseSensitive) {
            if (!smatch(mprGetPathExt(target), ext)) {
                target = sjoin(target, ".", ext, NULL);
            }
        } else {
            if (!scaselessmatch(mprGetPathExt(target), ext)) {
                target = sjoin(target, ".", ext, NULL);
            }
        }
    }
    /*
        See if module already loaded for this view
     */
    if (mprLookupKey(eroute->top->views, target)) {
        return target;
    }

#if !ME_STATIC
{
    MprPath info;
    cchar   *module, *path;

    path = mprJoinPath(route->documents, target);

    if (!eroute->combine) {
        /*
            If target exists as a cached module that has not yet been loaded
            Note: source may not be present.
         */
        module = getModuleName(route, "view", path);
        if (mprGetPathInfo(module, &info) == 0 && !info.isDir) {
            return target;
        }
    }

    /*
        If target exists as a view (extension appended above)
     */
    if (mprGetPathInfo(path, &info) == 0 && !info.isDir) {
        return target;
    }
}
#endif
    return 0;
}


/*
    Render a document by mapping a URL target to a document. The target is interpreted relative to route->documents.
    If target + defined extension exists, serve that.
    If target is a directory with an index.esp, return the index.esp without a redirect.
    If target is a directory without a trailing "/" but with an index.esp, do an external redirect to "URI/".
    Otherwise relay to the fileHandler.
    May yield.
 */
PUBLIC void espRenderDocument(HttpStream *stream, cchar *target)
{
    HttpUri     *up;
    MprKey      *kp;
    cchar       *dest;

    if (!target) {
        httpError(stream, HTTP_CODE_NOT_FOUND, "Cannot find document");
        return;
    }
    if (*target) {
        for (ITERATE_KEYS(stream->rx->route->extensions, kp)) {
            if (kp->data == HTTP->espHandler && kp->key && kp->key[0]) {
                if ((dest = checkView(stream, target, 0, kp->key)) != 0) {
                    httpLog(stream->trace, "esp.handler", "context", "msg:Render view %s", dest);
                    /* May yield */
                    espRenderView(stream, dest, 0);
                    return;
                }
            }
        }
    }
    /*
        Check for index
     */
    if ((dest = checkView(stream, target, "index", "esp")) != 0) {
        /*
            Must do external redirect first if URL does not end with "/"
         */
        if (!sends(stream->rx->parsedUri->path, "/")) {
            up = stream->rx->parsedUri;
            httpRedirect(stream, HTTP_CODE_MOVED_PERMANENTLY, httpFormatUri(up->scheme, up->host,
                up->port, sjoin(up->path, "/", NULL), up->reference, up->query, 0));
            return;
        }
        httpLog(stream->trace, "esp.handler", "context", "msg:Render index %s", dest);
        /* May yield */
        espRenderView(stream, dest, 0);
        return;
    }

    httpLog(stream->trace, "esp.handler", "context", "msg:Relay to the fileHandler");
    stream->rx->target = sclone(&stream->rx->pathInfo[1]);
    httpMapFile(stream);
    if (stream->tx->fileInfo.isDir) {
        httpHandleDirectory(stream);
    }
    if (!stream->tx->finalized) {
        httpSetFileHandler(stream, 0);
    }
}


/************************************ Support *********************************/
/*
    Create a per user session database clone.
    Used for demos so one users updates to not change anothers view of the database.
 */
static void pruneDatabases(Esp *esp)
{
    MprKey      *kp;

    lock(esp);
    for (ITERATE_KEYS(esp->databases, kp)) {
        if (!httpLookupSessionID(kp->key)) {
            mprRemoveKey(esp->databases, kp->key);
            /* Restart scan */
            kp = 0;
        }
    }
    unlock(esp);
}


/*
    This clones a database to give a private view per user.
 */
static int cloneDatabase(HttpStream *stream)
{
    Esp         *esp;
    EspRoute    *eroute;
    EspReq      *req;
    cchar       *id;

    req = stream->reqData;
    eroute = stream->rx->route->eroute;
    assert(eroute->edi);
    assert(eroute->edi->flags & EDI_PRIVATE);

    esp = req->esp;
    if (!esp->databases) {
        lock(esp);
        if (!esp->databases) {
            esp->databases = mprCreateHash(0, 0);
            esp->databasesTimer = mprCreateTimerEvent(NULL, "esp-databases", 60 * 1000, pruneDatabases, esp, 0);
        }
        unlock(esp);
    }
    /*
        If the user is logging in or out, this will create a redundant session here.
     */
    httpGetSession(stream, 1);
    id = httpGetSessionID(stream);
    if ((req->edi = mprLookupKey(esp->databases, id)) == 0) {
        if ((req->edi = ediClone(eroute->edi)) == 0) {
            mprLog("error esp", 0, "Cannot clone database: %s", eroute->edi->path);
            return MPR_ERR_CANT_OPEN;
        }
        mprAddKey(esp->databases, id, req->edi);
    }
    return 0;
}


#if !ME_STATIC
static cchar *getCacheName(HttpRoute *route, cchar *kind, cchar *target)
{
    EspRoute    *eroute;
    cchar       *appName, *canonical;

    eroute = route->eroute;
#if VXWORKS
    /*
        Trim the drive for VxWorks where simulated host drives only exist on the target
     */
    target = mprTrimPathDrive(target);
#endif
    canonical = mprGetPortablePath(mprGetRelPath(target, route->home));

    appName = eroute->appName;
    return eroute->combine ? appName : mprGetMD5WithPrefix(sfmt("%s:%s", appName, canonical), -1, sjoin(kind, "_", NULL));
}


static char *getModuleEntry(EspRoute *eroute, cchar *kind, cchar *source, cchar *cache)
{
    char    *cp, *entry;

    if (smatch(kind, "view")) {
        entry = sfmt("esp_%s", cache);

    } else if (smatch(kind, "app")) {
        if (eroute->combine) {
            entry = sfmt("esp_%s_%s_combine", kind, eroute->appName);
        } else {
            entry = sfmt("esp_%s_%s", kind, eroute->appName);
        }
    } else {
        /* Controller */
        if (eroute->appName) {
            entry = sfmt("esp_%s_%s_%s", kind, eroute->appName, mprTrimPathExt(mprGetPathBase(source)));
        } else {
            entry = sfmt("esp_%s_%s", kind, mprTrimPathExt(mprGetPathBase(source)));
        }
    }
    for (cp = entry; *cp; cp++) {
        if (!isalnum((uchar) *cp) && *cp != '_') {
            *cp = '_';
        }
    }
    return entry;
}


static cchar *getModuleName(HttpRoute *route, cchar *kind, cchar *target)
{
    cchar   *cacheDir, *cache;

    cache = getCacheName(route, "view", target);
    if ((cacheDir = httpGetDir(route, "CACHE")) == 0) {
        /* May not be set for non esp apps */
        cacheDir = "cache";
    }
    return mprJoinPathExt(mprJoinPaths(route->home, cacheDir, cache, NULL), ME_SHOBJ);
}


/*
    Load an ESP module. WARNING: this routine may yield. Take precautions to preserve the source argument so callers
    dont have to.
 */
static int espLoadModule(HttpRoute *route, MprDispatcher *dispatcher, cchar *kind, cchar *source, cchar **errMsg, bool *loaded)
{
    EspRoute    *eroute;
    MprModule   *mp;
    cchar       *cacheDir, *cache, *entry, *module;
    int         isView, recompile;

    eroute = route->eroute;
    *errMsg = "";
    assert(mprIsValid(source));
    route->source = source;

    if (loaded) {
        *loaded = 0;
    }
    cache = getCacheName(route, kind, source);
    if ((cacheDir = httpGetDir(route, "CACHE")) == 0) {
        cacheDir = "cache";
    }
    module = mprJoinPathExt(mprJoinPaths(route->home, cacheDir, cache, NULL), ME_SHOBJ);

    lock(esp);
    if (mprLookupModule(source) == 0 || eroute->update) {
        espModuleIsStale(route, source, module, &recompile);
        if (eroute->compile && mprPathExists(source, R_OK)) {
            isView = smatch(kind, "view");
            if (recompile || (isView && layoutIsStale(eroute, source, module))) {
                if (recompile) {
                    /*
                        WARNING: espCompile may yield. espCompile will retain the arguments (source, module, cache) for us.
                     */
                    if (!espCompile(route, dispatcher, source, module, cache, isView, (char**) errMsg)) {
                        unlock(esp);
                        return MPR_ERR_CANT_WRITE;
                    }
                }
            }
        }
    }
    if (mprLookupModule(source) == 0) {
        if (!mprPathExists(module, R_OK)) {
            *errMsg = sfmt("Module does not exist: %s", module);
            unlock(esp);
            return MPR_ERR_CANT_FIND;
        }
        entry = getModuleEntry(eroute, kind, source, cache);
        if ((mp = mprCreateModule(source, module, entry, route)) == 0) {
            *errMsg = "Memory allocation error loading module";
            unlock(esp);
            return MPR_ERR_MEMORY;
        }
        if (mprLoadModule(mp) < 0) {
            *errMsg = sfmt("Cannot load compiled esp module: %s", module);
            unlock(esp);
            return MPR_ERR_CANT_READ;
        }
        if (loaded) {
            *loaded = 1;
        }
    }
    unlock(esp);
    return 0;
}


/*
    Test if a module has been updated (is stale).
    This will unload the module if it loaded but stale.
    Set recompile to true if the source is absent or more recent.
    Will return false if the source does not exist (important for testing layouts).
 */
PUBLIC bool espModuleIsStale(HttpRoute *route, cchar *source, cchar *module, int *recompile)
{
    EspRoute    *eroute;
    MprModule   *mp;
    MprPath     sinfo, minfo;

    *recompile = 0;
    eroute = route->eroute;

    mprGetPathInfo(module, &minfo);
    if (!minfo.valid) {
        if ((mp = mprLookupModule(source)) != 0) {
            if (!espUnloadModule(source, ME_ESP_RELOAD_TIMEOUT)) {
                mprLog("error esp", 0, "Cannot unload module %s. Streams still open. Continue using old version.",
                    source);
                return 0;
            }
        }
        if (eroute->compile) {
            *recompile = 1;
            /* Module not loaded */
        }
        return 1;
    }
    if (eroute->compile) {
        mprGetPathInfo(source, &sinfo);
        if (sinfo.valid && sinfo.mtime > minfo.mtime) {
            if ((mp = mprLookupModule(source)) != 0) {
                if (!espUnloadModule(source, ME_ESP_RELOAD_TIMEOUT)) {
                    mprLog("warn esp", 4, "Cannot unload module %s. Streams still open. Continue using old version.",
                        source);
                    return 0;
                }
            }
            *recompile = 1;
            mprLog("info esp", 4, "Source %s is newer than module %s, recompiling ...", source, module);
            return 1;
        }
    }
    if ((mp = mprLookupModule(source)) != 0) {
        if (minfo.mtime > mp->modified) {
            /* Module file has been updated */
            if (!espUnloadModule(source, ME_ESP_RELOAD_TIMEOUT)) {
                mprLog("warn esp", 4, "Cannot unload module %s. Streams still open. Continue using old version.",
                    source);
                return 0;
            }
            mprLog("info esp", 4, "Module %s has been externally updated, reloading ...", module);
            return 1;
        }
    }
    /* Loaded module is current */
    return 0;
}


/*
    Check if the layout has changed. Returns false if the layout does not exist.
 */
static bool layoutIsStale(EspRoute *eroute, cchar *source, cchar *module)
{
    char    *data, *lpath, *quote;
    cchar   *layout, *layoutsDir;
    ssize   len;
    bool    stale;
    int     recompile;

    stale = 0;
    layoutsDir = httpGetDir(eroute->route, "LAYOUTS");
    if ((data = mprReadPathContents(source, &len)) != 0) {
        if ((lpath = scontains(data, "@ layout \"")) != 0) {
            lpath = strim(&lpath[10], " ", MPR_TRIM_BOTH);
            if ((quote = schr(lpath, '"')) != 0) {
                *quote = '\0';
            }
            layout = (layoutsDir && *lpath) ? mprJoinPath(layoutsDir, lpath) : 0;
        } else {
            layout = (layoutsDir) ? mprJoinPath(layoutsDir, "default.esp") : 0;
        }
        if (layout) {
            stale = espModuleIsStale(eroute->route, layout, module, &recompile);
            if (stale) {
                mprLog("info esp", 4, "esp layout %s is newer than module %s", layout, module);
            }
        }
    }
    return stale;
}
#else

PUBLIC bool espModuleIsStale(HttpRoute *route, cchar *source, cchar *module, int *recompile)
{
    return 0;
}
#endif /* ME_STATIC */


/************************************ Esp Route *******************************/
/*
    Public so that esp.c can also call
 */
PUBLIC void espManageEspRoute(EspRoute *eroute, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(eroute->actions);
        mprMark(eroute->appName);
        mprMark(eroute->compileCmd);
        mprMark(eroute->configFile);
        mprMark(eroute->currentSession);
        mprMark(eroute->edi);
        mprMark(eroute->env);
        mprMark(eroute->linkCmd);
        mprMark(eroute->searchPath);
        mprMark(eroute->top);
        mprMark(eroute->views);
        mprMark(eroute->winsdk);
#if DEPRECATED
        mprMark(eroute->combineScript);
        mprMark(eroute->combineSheet);
#endif
    }
}


PUBLIC EspRoute *espCreateRoute(HttpRoute *route)
{
    EspRoute    *eroute;

    if ((eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return 0;
    }
    route->eroute = eroute;
    eroute->route = route;
    eroute->compile = 1;
    eroute->keep = 0;
    eroute->update = 1;
    eroute->compileMode = ESP_COMPILE_SYMBOLS;

    if (route->parent && route->parent->eroute) {
        eroute->top = ((EspRoute*) route->parent->eroute)->top;
    } else {
        eroute->top = eroute;
    }
    eroute->appName = sclone("app");
    return eroute;
}


static EspRoute *cloneEspRoute(HttpRoute *route, EspRoute *parent)
{
    EspRoute      *eroute;

    assert(parent);
    assert(route);

    if ((eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return 0;
    }
    eroute->route = route;
    eroute->top = parent->top;
    eroute->searchPath = parent->searchPath;
    eroute->configFile = parent->configFile;
    eroute->edi = parent->edi;
    eroute->commonController = parent->commonController;
    if (parent->compileCmd) {
        eroute->compileCmd = sclone(parent->compileCmd);
    }
    if (parent->linkCmd) {
        eroute->linkCmd = sclone(parent->linkCmd);
    }
    if (parent->env) {
        eroute->env = mprCloneHash(parent->env);
    }
    eroute->appName = parent->appName;
    eroute->combine = parent->combine;
    eroute->compile = parent->compile;
    eroute->keep = parent->keep;
    eroute->update = parent->update;
#if DEPRECATED
    eroute->combineScript = parent->combineScript;
    eroute->combineSheet = parent->combineSheet;
#endif
    route->eroute = eroute;
    return eroute;
}


/*
    Get an EspRoute. Allocate if required.
    It is expected that the caller will modify the EspRoute.
 */
PUBLIC EspRoute *espRoute(HttpRoute *route, bool create)
{
    HttpRoute   *rp;

    if (route->eroute && (!route->parent || route->parent->eroute != route->eroute)) {
        return route->eroute;
    }
    /*
        Lookup up the route chain for any configured EspRoutes to clone
     */
    for (rp = route; rp; rp = rp->parent) {
        if (rp->eroute) {
            return cloneEspRoute(route, rp->eroute);
        }
        if (rp->parent == 0 && create) {
            /*
                Create an ESP configuration on the top level parent so others can inherit
                Load the compiler rules once for all
             */
            rp->eroute = espCreateRoute(rp);
            break;
        }
    }
    if (rp) {
        return cloneEspRoute(route, rp->eroute);
    }
    return 0;
}


/*
    Manage all links for EspReq for the garbage collector
 */
static void manageReq(EspReq *req, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(req->commandLine);
        mprMark(req->data);
        mprMark(req->edi);
        mprMark(req->feedback);
        mprMark(req->lastFeedback);
        mprMark(req->route);
    }
}


/*
    Manage all links for Esp for the garbage collector
 */
static void manageEsp(Esp *esp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(esp->databases);
        mprMark(esp->databasesTimer);
        mprMark(esp->ediService);
        mprMark(esp->internalOptions);
        mprMark(esp->local);
        mprMark(esp->mutex);
        mprMark(esp->vstudioEnv);
        mprMark(esp->hostedDocuments);
    }
}

/*********************************** Directives *******************************/
/*
    Load the ESP configuration file esp.json (eroute->configFile) and an optional pak.json file
    WARNING: may yield
 */
PUBLIC int espLoadConfig(HttpRoute *route)
{
    EspRoute    *eroute;
    HttpRoute   *rp;
    cchar       *cookie, *home, *name, *package;
    int         next;
    bool        modified;

    eroute = route->eroute;
    if (!eroute) {
        mprLog("esp error", 0, "Cannot find esp configuration when loading config");
        return MPR_ERR_CANT_LOAD;
    }
    if (eroute->loaded && !eroute->update) {
        return 0;
    }
    home = eroute->configFile ? mprGetPathDir(eroute->configFile) : route->home;
    package = mprJoinPath(home, "pak.json");
    modified = 0;
    ifConfigModified(route, eroute->configFile, &modified);
    ifConfigModified(route, package, &modified);

    if (modified) {
        lock(esp);
        httpInitConfig(route);
        if (mprPathExists(package, R_OK) && !mprSamePath(package, eroute->configFile)) {
            if (httpLoadConfig(route, package) < 0) {
                unlock(esp);
                return MPR_ERR_CANT_LOAD;
            }
        }
        if (httpLoadConfig(route, eroute->configFile) < 0) {
            unlock(esp);
            return MPR_ERR_CANT_LOAD;
        }
        if ((name = espGetConfig(route, "name", 0)) != 0) {
            eroute->appName = sreplace(name, "-", "_");
        }
        eroute->encodeTypes = smatch(espGetConfig(route, "esp.encodeTypes", NULL), "true");

        cookie = sfmt("esp-%s", eroute->appName);
        for (ITERATE_ITEMS(route->host->routes, rp, next)) {
            if (!rp->cookie) {
                httpSetRouteCookie(rp, cookie);
            }
        }
        unlock(esp);
    }
    if (!httpGetDir(route, "CACHE")) {
        espSetDefaultDirs(route, 0);
    }
    return 0;
}


/*
    Preload application module
    WARNING: may yield when compiling modules
 */
static bool preload(HttpRoute *route)
{
#if !ME_STATIC
    EspRoute    *eroute;
    MprJson     *preload, *item, *sources, *si;
    MprList     *files;
    cchar       *errMsg, *source;
    char        *kind;
    int         i, index, next;

    eroute = route->eroute;
    if (eroute->app && !(route->flags & HTTP_ROUTE_NO_LISTEN)) {
        if (eroute->combine) {
            /* Note: source file does not have to be present - the name is used to calculate the module name */
            source = mprJoinPaths(route->home, httpGetDir(route, "CACHE"), sfmt("%s.c", eroute->appName), NULL);
            if (espLoadModule(route, NULL, "app", source, &errMsg, NULL) < 0) {
                mprLog("error esp", 0, "%s", errMsg);
                return 0;
            }
        } else {
            if ((sources = mprGetJsonObj(route->config, "esp.app.source")) != 0) {
                for (ITERATE_JSON(sources, si, index)) {
                    files = mprGlobPathFiles(route->home, si->value, 0);
                    if (mprGetListLength(files) == 0) {
                        mprLog("error esp", 0, "ESP source pattern does not match any files \"%s\"", si->value);
                    }
                    for (ITERATE_ITEMS(files, source, next)) {
                        /* May yield */
                        route->source = source;
                        if (espLoadModule(route, NULL, "app", source, &errMsg, NULL) < 0) {
                            mprLog("error esp", 0, "%s", errMsg);
                            return 0;
                        }
                    }
                }
            } else {
                /*
                    DEPRECATE - load a src/app.c
                 */
                source = mprJoinPaths(route->home, httpGetDir(route, "SRC"), "app.c", NULL);
                if (mprPathExists(source, R_OK)) {
                    /* May yield */
                    route->source = source;
                    mprLog("info esp", 0, "Specify app.c in esp.app.source: ['app.c']");
                    if (espLoadModule(route, NULL, "app", source, &errMsg, NULL) < 0) {
                        mprLog("error esp", 0, "%s", errMsg);
                        return 0;
                    }
                }
            }
        }

        if (!eroute->combine && (preload = mprGetJsonObj(route->config, "esp.preload")) != 0) {
            for (ITERATE_JSON(preload, item, i)) {
                source = ssplit(sclone(item->value), ":", &kind);
                if (*kind == '\0') {
                    kind = "controller";
                }
                source = mprJoinPaths(route->home, httpGetDir(route, "CONTROLLERS"), source, NULL);
                /* May yield */
                route->source = source;
                if (espLoadModule(route, NULL, kind, source, &errMsg, NULL) < 0) {
                    mprLog("error esp", 0, "Cannot preload esp module %s. %s", source, errMsg);
                    return 0;
                }
            }
        }
        mprLog("esp info", 6, "Loaded ESP application \"%s\", profile \"%s\" with options: combine %d, compile %d, compile mode %d, update %d",
            eroute->appName, route->mode ? route->mode : "unset", eroute->combine, eroute->compile, eroute->compileMode, eroute->update);
    }
#endif
    return 1;
}


/*
    Initialize ESP.
    Prefix is the URI prefix for the application.
    Path is the path to the esp.json.
 */
PUBLIC int espInit(HttpRoute *route, cchar *prefix, cchar *path)
{
    EspRoute    *eroute;
    cchar       *hostname;
    bool        yielding;

    if (!route) {
        return MPR_ERR_BAD_ARGS;
    }
    lock(esp);

    eroute = espRoute(route, 1);

    if (prefix) {
        if (*prefix != '/') {
            prefix = sjoin("/", prefix, NULL);
        }
        prefix = stemplate(prefix, route->vars);
        httpSetRoutePrefix(route, prefix);
        httpSetRoutePattern(route, sfmt("^%s", prefix), 0);
        hostname = route->host->name ? route->host->name : "default";
        mprLog("info esp", 3, "Load ESP app: %s%s from %s", hostname, prefix, path);
    }
    eroute->top = eroute;
    if (path) {
        if (mprPathExists(path, R_OK)) {
            httpSetRouteHome(route, mprGetPathDir(path));
            eroute->configFile = sclone(path);
        } else {
            mprLog("error esp", 0, "Cannot locate ESP config file: %s", path);
            return MPR_ERR_CANT_LOAD;
        }
    }
    if (!route->handler) {
        httpAddRouteHandler(route, "espHandler", "esp");
    }
    /*
        Loading config may run commands. To make it easier for parsing code, we disable GC by not consenting to
        yield for this section. This should only happen on application load.
     */
    yielding = mprSetThreadYield(NULL, 0);
    if (espLoadConfig(route) < 0) {
        mprSetThreadYield(NULL, yielding);
        unlock(esp);
        return MPR_ERR_CANT_LOAD;
    }
    if (eroute->compile && espLoadCompilerRules(route) < 0) {
        mprSetThreadYield(NULL, yielding);
        unlock(esp);
        return MPR_ERR_CANT_OPEN;
    }
    if (route->database && !eroute->edi && espOpenDatabase(route, route->database) < 0) {
        mprLog("error esp", 0, "Cannot open database %s", route->database);
        mprSetThreadYield(NULL, yielding);
        unlock(esp);
        return MPR_ERR_CANT_LOAD;
    }
    if (!preload(route)) {
        mprSetThreadYield(NULL, yielding);
        unlock(esp);
        return MPR_ERR_CANT_LOAD;
    }
    mprSetThreadYield(NULL, yielding);
    unlock(esp);
    return 0;
}


PUBLIC int espOpenDatabase(HttpRoute *route, cchar *spec)
{
    EspRoute    *eroute;
    char        *provider, *path, *dir;
    int         flags;

    eroute = route->eroute;
    if (eroute->edi) {
        return 0;
    }
    flags = EDI_CREATE | EDI_AUTO_SAVE;
    if (smatch(spec, "default")) {
#if ME_COM_SQLITE
        spec = sfmt("sdb://%s.sdb", eroute->appName);
#elif ME_COM_MDB
        spec = sfmt("mdb://%s.mdb", eroute->appName);
#else
        mprLog("error esp", 0, "No database handler configured (MDB or SQLITE). Reconfigure.");
        return MPR_ERR_BAD_ARGS;
#endif
    }
    provider = ssplit(sclone(spec), "://", &path);
    if (*provider == '\0' || *path == '\0') {
        return MPR_ERR_BAD_ARGS;
    }
    path = mprJoinPaths(route->home, httpGetDir(route, "DB"), path, NULL);
    dir = mprGetPathDir(path);
    if (!mprPathExists(dir, X_OK)) {
        mprMakeDir(dir, 0755, -1, -1, 1);
    }
    if ((eroute->edi = ediOpen(mprGetRelPath(path, NULL), provider, flags)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    route->database = sclone(spec);
    return 0;
}


PUBLIC void espCloseDatabase(HttpRoute *route)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    if (eroute->edi) {
        ediClose(eroute->edi);
        eroute->edi = 0;
    }
}


PUBLIC int espReloadDatabase(HttpRoute *route)
{
    if (route->database) {
        espCloseDatabase(route);
        return espOpenDatabase(route, route->database);
    }
    return 0;
}


static void setDir(HttpRoute *route, cchar *key, cchar *value, bool force)
{
    if (force) {
        httpSetDir(route, key, value);
    } else if (!httpGetDir(route, key)) {
        httpSetDir(route, key, value);
    }
}


PUBLIC void espSetDefaultDirs(HttpRoute *route, bool force)
{
    cchar   *controllers, *documents, *path, *migrations;

    documents = mprJoinPath(route->home, "dist");
    /*
        Consider keeping documents, web and public
     */
    if (!mprPathExists(documents, X_OK)) {
        documents = mprJoinPath(route->home, "documents");
        if (!mprPathExists(documents, X_OK)) {
            documents = mprJoinPath(route->home, "web");
            if (!mprPathExists(documents, X_OK)) {
                documents = mprJoinPath(route->home, "client");
                if (!mprPathExists(documents, X_OK)) {
                    documents = mprJoinPath(route->home, "public");
                    if (!mprPathExists(documents, X_OK)) {
#if ME_APPWEB_PRODUCT
                        if (!esp->hostedDocuments && mprPathExists("install.conf", R_OK)) {
                            /*
                                This returns the documents directory of the default route of the default host
                                When Appweb switches to appweb.json, then just it should be loaded with pak.json
                             */
                            char *output;
                            bool yielding = mprSetThreadYield(NULL, 0);
                            if (mprRun(NULL, "appweb --show-documents", NULL, (char**) &output, NULL, 5000) == 0) {
                                documents = esp->hostedDocuments = strim(output, "\n", MPR_TRIM_END);
                            } else {
                                documents = route->home;
                            }
                            mprSetThreadYield(NULL, yielding);
                        } else
#endif
                        {
                            documents = route->home;
                        }
                    }
                }
            }
        }
    }

    /*
        Detect if a controllers directory exists. Set controllers to "." if absent.
     */
    controllers = "controllers";
    path = mprJoinPath(route->home, controllers);
    if (!mprPathExists(path, X_OK)) {
        controllers = ".";
    }

    migrations = "db/migrations";
    path = mprJoinPath(route->home, migrations);
    if (!mprPathExists(path, X_OK)) {
        migrations = "migrations";
    }
    force = 0;
    setDir(route, "DOCUMENTS", documents, force);
    setDir(route, "HOME", route->home, force);
    setDir(route, "MIGRATIONS", migrations, force);
    setDir(route, "UPLOAD", "/tmp", 0);

    setDir(route, "CACHE", 0, force);
    setDir(route, "CONTROLLERS", controllers, force);
    setDir(route, "CONTENTS", 0, force);
    setDir(route, "DB", 0, force);
    setDir(route, "LAYOUTS", 0, force);
    setDir(route, "LIB", 0, force);
    setDir(route, "PAKS", 0, force);
    setDir(route, "PARTIALS", 0, force);
    setDir(route, "SRC", 0, force);
}


/*
    Initialize and load a statically linked ESP module
 */
PUBLIC int espStaticInitialize(EspModuleEntry entry, cchar *appName, cchar *routeName)
{
    HttpRoute   *route;

    if ((route = httpLookupRoute(NULL, routeName)) == 0) {
        mprLog("error esp", 0, "Cannot find route %s", routeName);
        return MPR_ERR_CANT_ACCESS;
    }
    return (entry)(route, NULL);
}


PUBLIC int espBindProc(HttpRoute *parent, cchar *pattern, void *proc)
{
    HttpRoute   *route;
    EspRoute    *eroute;

    if ((route = httpDefineRoute(parent, "ALL", pattern, "$&", "unused")) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    httpSetRouteHandler(route, "espHandler");
    espAction(route, pattern, NULL, proc);
    eroute = route->eroute;
    eroute->update = 0;
    return 0;
}


static void ifConfigModified(HttpRoute *route, cchar *path, bool *modified)
{
    EspRoute    *eroute;
    MprPath     info;

    if (path) {
        eroute = route->eroute;
        mprGetPathInfo(path, &info);
        if (info.mtime > eroute->loaded) {
            *modified = 1;
            eroute->loaded = info.mtime;
        }
    }
}


/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    espTemplate.c -- ESP templated web pages with embedded C code.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/



/************************************ Defines *********************************/
/*
      ESP lexical analyser tokens
 */
#define ESP_TOK_ERR            -1            /* Any input error */
#define ESP_TOK_EOF             0            /* End of file */
#define ESP_TOK_CODE            1            /* <% text %> */
#define ESP_TOK_EXPR            2            /* <%= expression %> */
#define ESP_TOK_CONTROL         3            /* <%^ control */
#define ESP_TOK_PARAM           4            /* %$param */
#define ESP_TOK_FIELD           5            /* %#field */
#define ESP_TOK_VAR             6            /* %!var */
#define ESP_TOK_HOME            7            /* %~ Home URL */
#define ESP_TOK_LITERAL         8            /* literal HTML */
#if DEPRECATED || 1
#define ESP_TOK_SERVER          9            /* %| Server URL  */
#endif

/*
    ESP page parser structure
 */
typedef struct EspParse {
    int     lineNumber;                     /**< Line number for error reporting */
    char    *data;                          /**< Input data to parse */
    char    *next;                          /**< Next character in input */
    cchar   *path;                          /**< Filename being parsed */
    MprBuf  *token;                         /**< Input token */
} EspParse;


typedef struct CompileContext {
    cchar   *csource;
    cchar   *source;
    cchar   *module;
    cchar   *cache;
} CompileContext;

/************************************ Forwards ********************************/

static CompileContext* allocContext(cchar *source, cchar *csource, cchar *module, cchar *cache);
static int getEspToken(EspParse *parse);
static cchar *getDebug(EspRoute *eroute);
static cchar *getEnvString(HttpRoute *route, cchar *key, cchar *defaultValue);
static cchar *getArExt(cchar *os);
static cchar *getShlibExt(cchar *os);
static cchar *getShobjExt(cchar *os);
static cchar *getArPath(cchar *os, cchar *arch);
static cchar *getCompilerName(cchar *os, cchar *arch);
static cchar *getCompilerPath(cchar *os, cchar *arch);
static cchar *getLibs(cchar *os);
static cchar *getMappedArch(cchar *arch);
static cchar *getObjExt(cchar *os);
static cchar *getVxCPU(cchar *arch);
static void manageContext(CompileContext *context, int flags);
static bool matchToken(cchar **str, cchar *token);

#if ME_WIN_LIKE
static cchar *getWinSDK(HttpRoute *route);
static cchar *getWinVer(HttpRoute *route);
#endif

/************************************* Code ***********************************/
/*
    Tokens:
    APPINC      Application include directory
    AR          Library archiver (ar)
    ARLIB       Archive library extension (.a, .lib)
    ARCH        Build architecture (64)
    CC          Compiler (cc)
    DEBUG       Debug compilation options (-g, -Zi -Od)
    GCC_ARCH    ARCH mapped to gcc -arch switches (x86_64)
    INC         Include directory build/platform/inc
    LIBPATH     Library search path
    LIBS        Libraries required to link with ESP
    OBJ         Name of compiled source (out/lib/view-MD5.o)
    MOD         Output module (view_MD5)
    SHLIB       Host Shared library (.lib, .so)
    SHOBJ       Host Shared Object (.dll, .so)
    SRC         Source code for view or controller (already templated)
    TMP         Temp directory
    VS          Visual Studio directory
    WINSDK      Windows SDK directory
 */
PUBLIC char *espExpandCommand(HttpRoute *route, cchar *command, cchar *source, cchar *module)
{
    MprBuf      *buf;
    Http        *http;
    EspRoute    *eroute;
    cchar       *cp, *outputModule, *os, *arch, *profile, *srcDir;
    char        *tmp;

    if (command == 0) {
        return 0;
    }
    http = MPR->httpService;
    eroute = route->eroute;
    outputModule = mprTrimPathExt(module);
    httpParsePlatform(http->platform, &os, &arch, &profile);
    buf = mprCreateBuf(-1, -1);

    for (cp = command; *cp; ) {
        if (*cp == '$') {
            if (matchToken(&cp, "${ARCH}")) {
                /* Target architecture (x86|mips|arm|x64) */
                mprPutStringToBuf(buf, arch);

            } else if (matchToken(&cp, "${ARLIB}")) {
                /* .a, .lib */
                mprPutStringToBuf(buf, getArExt(os));

            } else if (matchToken(&cp, "${GCC_ARCH}")) {
                /* Target architecture mapped to GCC mtune|mcpu values */
                mprPutStringToBuf(buf, getMappedArch(arch));

            } else if (matchToken(&cp, "${APPINC}")) {
                /* Application src include directory */
                if ((srcDir = httpGetDir(route, "SRC")) == 0) {
                    srcDir = ".";
                }
                srcDir = getEnvString(route, "APPINC", srcDir);
                mprPutStringToBuf(buf, srcDir);

            } else if (matchToken(&cp, "${INC}")) {
                /* Include directory for the configuration */
                mprPutStringToBuf(buf, mprJoinPath(http->platformDir, "inc"));

            } else if (matchToken(&cp, "${LIBPATH}")) {
                /* Library directory for Appweb libraries for the target */
                mprPutStringToBuf(buf, mprJoinPath(http->platformDir, "bin"));

            } else if (matchToken(&cp, "${LIBS}")) {
                /* Required libraries to link. These may have nested ${TOKENS} */
                mprPutStringToBuf(buf, espExpandCommand(route, getLibs(os), source, module));

            } else if (matchToken(&cp, "${MOD}")) {
                /* Output module path in the cache without extension */
                mprPutStringToBuf(buf, outputModule);

            } else if (matchToken(&cp, "${OBJ}")) {
                /* Output object with extension (.o) in the cache directory */
                mprPutStringToBuf(buf, mprJoinPathExt(outputModule, getObjExt(os)));

            } else if (matchToken(&cp, "${OS}")) {
                /* Target architecture (freebsd|linux|macosx|windows|vxworks) */
                mprPutStringToBuf(buf, os);

            } else if (matchToken(&cp, "${SHLIB}")) {
                /* .dll, .so, .dylib */
                mprPutStringToBuf(buf, getShlibExt(os));

            } else if (matchToken(&cp, "${SHOBJ}")) {
                /* .dll, .so, .dylib */
                mprPutStringToBuf(buf, getShobjExt(os));

            } else if (matchToken(&cp, "${SRC}")) {
                /* View (already parsed into C code) or controller source */
                mprPutStringToBuf(buf, source);

            } else if (matchToken(&cp, "${TMP}")) {
                if ((tmp = getenv("TMPDIR")) == 0) {
                    if ((tmp = getenv("TMP")) == 0) {
                        tmp = getenv("TEMP");
                    }
                }
                mprPutStringToBuf(buf, tmp ? tmp : ".");

#if ME_WIN_LIKE
            } else if (matchToken(&cp, "${VS}")) {
                mprPutStringToBuf(buf, espGetVisualStudio());
            } else if (matchToken(&cp, "${WINSDK}")) {
                mprPutStringToBuf(buf, getWinSDK(route));
            } else if (matchToken(&cp, "${WINVER}")) {
                mprPutStringToBuf(buf, getWinVer(route));
#endif

            } else if (matchToken(&cp, "${VXCPU}")) {
                mprPutStringToBuf(buf, getVxCPU(arch));

            /*
                These vars can be also be configured from environment variables.
                NOTE: the default esp.conf includes "esp->vxworks.conf" which has EspEnv definitions for the
                configured VxWorks toolchain.
             */
            } else if (matchToken(&cp, "${AR}")) {
                mprPutStringToBuf(buf, getEnvString(route, "AR", getArPath(os, arch)));

            } else if (matchToken(&cp, "${CC}")) {
                mprPutStringToBuf(buf, getEnvString(route, "CC", getCompilerPath(os, arch)));

            } else if (matchToken(&cp, "${CFLAGS}")) {
                mprPutStringToBuf(buf, getEnvString(route, "CFLAGS", ""));

            } else if (matchToken(&cp, "${DEBUG}")) {
                mprPutStringToBuf(buf, getEnvString(route, "DEBUG", getDebug(eroute)));

            } else if (matchToken(&cp, "${LDFLAGS}")) {
                mprPutStringToBuf(buf, getEnvString(route, "LDFLAGS", ""));

            } else if (matchToken(&cp, "${LIB}")) {
                mprPutStringToBuf(buf, getEnvString(route, "LIB", ""));

            } else if (matchToken(&cp, "${LINK}")) {
                mprPutStringToBuf(buf, getEnvString(route, "LINK", ""));

            } else if (matchToken(&cp, "${WIND_BASE}")) {
                mprPutStringToBuf(buf, getEnvString(route, "WIND_BASE", WIND_BASE));

            } else if (matchToken(&cp, "${WIND_HOME}")) {
                mprPutStringToBuf(buf, getEnvString(route, "WIND_HOME", WIND_HOME));

            } else if (matchToken(&cp, "${WIND_HOST_TYPE}")) {
                mprPutStringToBuf(buf, getEnvString(route, "WIND_HOST_TYPE", WIND_HOST_TYPE));

            } else if (matchToken(&cp, "${WIND_PLATFORM}")) {
                mprPutStringToBuf(buf, getEnvString(route, "WIND_PLATFORM", WIND_PLATFORM));

            } else if (matchToken(&cp, "${WIND_GNU_PATH}")) {
                mprPutStringToBuf(buf, getEnvString(route, "WIND_GNU_PATH", WIND_GNU_PATH));

            } else if (matchToken(&cp, "${WIND_CCNAME}")) {
                mprPutStringToBuf(buf, getEnvString(route, "WIND_CCNAME", getCompilerName(os, arch)));

            } else {
                mprPutCharToBuf(buf, *cp++);
            }
        } else {
            mprPutCharToBuf(buf, *cp++);
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


static int runCommand(HttpRoute *route, MprDispatcher *dispatcher, cchar *command, cchar *csource, cchar *module,
    char **errMsg)
{
    MprCmd      *cmd;
    MprKey      *var;
    MprList     *elist;
    EspRoute    *eroute;
    cchar       **env, *commandLine;
    char        *err, *out;

    *errMsg = 0;
    eroute = route->eroute;
    if ((commandLine = espExpandCommand(route, command, csource, module)) == 0) {
        *errMsg = sfmt("Missing EspCompile directive for %s", csource);
        return MPR_ERR_CANT_READ;
    }
    mprLog("info esp run", 4, "%s", commandLine);
    if (eroute->env) {
        elist = mprCreateList(0, MPR_LIST_STABLE);
        for (ITERATE_KEYS(eroute->env, var)) {
            mprAddItem(elist, sfmt("%s=%s", var->key, (char*) var->data));
        }
        mprAddNullItem(elist);
        env = (cchar**) &elist->items[0];
    } else {
        env = 0;
    }
    cmd = mprCreateCmd(dispatcher);
    if (eroute->searchPath) {
        mprSetCmdSearchPath(cmd, eroute->searchPath);
    }
    /*
        WARNING: GC will run here
     */
    if (mprRunCmd(cmd, commandLine, env, NULL, &out, &err, -1, 0) != 0) {
        if (err == 0 || *err == '\0') {
            /* Windows puts errors to stdout Ugh! */
            err = out;
        }
        mprLog("error esp", 0, "Cannot run command: %s, error %s", command, err);
        if (route->flags & HTTP_ROUTE_SHOW_ERRORS) {
            *errMsg = sfmt("Cannot run command: %s, error %s", command, err);
        } else {
            *errMsg = "Cannot compile view";
        }
        mprDestroyCmd(cmd);
        return MPR_ERR_CANT_COMPLETE;
    }
    mprDestroyCmd(cmd);
    return 0;
}


PUBLIC int espLoadCompilerRules(HttpRoute *route)
{
    EspRoute    *eroute;
    cchar       *compile, *rules;

    eroute = route->eroute;
    if (eroute->compileCmd) {
        return 0;
    }
    if ((compile = mprGetJson(route->config, "esp.rules")) == 0) {
        compile = ESP_COMPILE_JSON;
    }
    rules = mprJoinPath(mprGetAppDir(), compile);
    if (mprPathExists(rules, R_OK) && httpLoadConfig(route, rules) < 0) {
        mprLog("error esp", 0, "Cannot parse %s", rules);
        return MPR_ERR_CANT_OPEN;
    }
    return 0;
}


/*
    Compile a view or controller

    cacheName   MD5 cache name (not a full path)
    source      ESP source file name
    module      Module file name

    WARNING: this routine yields and runs GC. All parameters must be retained by the caller.
 */
PUBLIC bool espCompile(HttpRoute *route, MprDispatcher *dispatcher, cchar *source, cchar *module, cchar *cacheName,
    int isView, char **errMsg)
{
    MprFile         *fp;
    EspRoute        *eroute;
    CompileContext  *context;
    cchar           *csource, *layoutsDir;
    char            *layout, *script, *page, *err;
    ssize           len;

    eroute = route->eroute;
    assert(eroute->compile);

    layout = 0;
    *errMsg = 0;

    mprLog("info esp", 2, "Compile %s", source);
    if (isView) {
        if ((page = mprReadPathContents(source, &len)) == 0) {
            *errMsg = sfmt("Cannot read %s", source);
            return 0;
        }
#if DEPRECATED || 1
        if ((layoutsDir = httpGetDir(route, "LAYOUTS")) != 0) {
            layout = mprJoinPath(layoutsDir, "default.esp");
        }
#endif
        if ((script = espBuildScript(route, page, source, cacheName, layout, NULL, &err)) == 0) {
            *errMsg = sfmt("Cannot build: %s, error: %s", source, err);
            return 0;
        }
        csource = mprJoinPathExt(mprTrimPathExt(module), ".c");
        mprMakeDir(mprGetPathDir(csource), 0775, -1, -1, 1);
        if ((fp = mprOpenFile(csource, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
            *errMsg = sfmt("Cannot open compiled script file %s", csource);
            return 0;
        }
        len = slen(script);
        if (mprWriteFile(fp, script, len) != len) {
            *errMsg = sfmt("Cannot write compiled script file %s", csource);
            mprCloseFile(fp);
            return 0;
        }
        mprCloseFile(fp);
    } else {
        csource = source;
    }
    mprMakeDir(mprGetPathDir(module), 0775, -1, -1, 1);

#if ME_WIN_LIKE
    {
        /*
            Force a clean windows compile by removing the obj, pdb and ilk files
         */
        cchar   *path;
        path = mprReplacePathExt(module, "obj");
        if (mprPathExists(path, F_OK)) {
            mprDeletePath(path);
        }
        path = mprReplacePathExt(module, "pdb");
        if (mprPathExists(path, F_OK)) {
            mprDeletePath(path);
        }
        path = mprReplacePathExt(module, "ilk");
        if (mprPathExists(path, F_OK)) {
            mprDeletePath(path);
        }
    }
#endif

    if (!eroute->compileCmd && espLoadCompilerRules(route) < 0) {
        return 0;
    }

    context = allocContext(source, csource, module, cacheName);
    mprAddRoot(context);

    /*
        Run compiler: WARNING: GC yield here
     */
    if (runCommand(route, dispatcher, eroute->compileCmd, csource, module, errMsg) != 0) {
        mprRemoveRoot(context);
        return 0;
    }
    if (eroute->linkCmd) {
        /* WARNING: GC yield here */
        if (runCommand(route, dispatcher, eroute->linkCmd, csource, module, errMsg) != 0) {
            mprRemoveRoot(context);
            return 0;
        }
#if !MACOSX
        /*
            MAC needs the object for debug information
         */
        mprDeletePath(mprJoinPathExt(mprTrimPathExt(module), &ME_OBJ[1]));
#endif
    }
#if ME_WIN_LIKE
    {
        /*
            Windows leaves intermediate object in the current directory
         */
        cchar *path = mprReplacePathExt(mprGetPathBase(csource), "obj");
        if (mprPathExists(path, F_OK)) {
            mprDeletePath(path);
        }
    }
#endif
    if (!eroute->keep && isView) {
        mprDeletePath(csource);
    }
    mprRemoveRoot(context);
    return 1;
}


static char *fixMultiStrings(cchar *str)
{
    cchar   *cp;
    char    *buf, *bp;
    ssize   len;
    int     count, quoted;

    for (count = 0, cp = str; *cp; cp++) {
        if (*cp == '\n' || *cp == '"') {
            count++;
        }
    }
    len = slen(str);
    if ((buf = mprAlloc(len + (count * 3) + 1)) == 0) {
        return 0;
    }
    quoted = 0;
    for (cp = str, bp = buf; *cp; cp++) {
        if (*cp == '`') {
            *bp++ = '"';
            quoted = !quoted;
        } else if (quoted) {
            if (*cp == '\n') {
                *bp++ = '\\';
            } else if (*cp == '"') {
                *bp++ = '\\';
            } else if (*cp == '\\' && cp[1] != '\\') {
                /* Ignore */ ;
            }
            *bp++ = *cp;
        } else {
            *bp++ = *cp;
        }
    }
    *bp = '\0';
    return buf;
}


static char *joinLine(cchar *str, ssize *lenp)
{
    cchar   *cp;
    char    *buf, *bp;
    ssize   len;
    int     count, bquote;

    for (count = 0, cp = str; *cp; cp++) {
        if (*cp == '\n' || *cp == '\r') {
            count++;
        }
    }
    /*
        Allocate room to backquote newlines (requires 3)
     */
    len = slen(str);
    if ((buf = mprAlloc(len + (count * 3) + 1)) == 0) {
        return 0;
    }
    bquote = 0;
    for (cp = str, bp = buf; *cp; cp++) {
        if (*cp == '\n') {
            *bp++ = '\\';
            *bp++ = 'n';
            *bp++ = '\\';
        } else if (*cp == '\r') {
            *bp++ = '\\';
            *bp++ = 'r';
            continue;
        } else if (*cp == '\\') {
            if (cp[1]) {
                *bp++ = *cp++;
                bquote++;
            }
        }
        *bp++ = *cp;
    }
    *bp = '\0';
    *lenp = len - bquote;
    return buf;
}


/*
    Convert an ESP web page into C code
    Directives:
        <%                  Begin esp section containing C code
        <%=                 Begin esp section containing an expression to evaluate and substitute
        <%= [%fmt]          Begin a formatted expression to evaluate and substitute. Format is normal printf format.
                            Use %S for safe HTML escaped output.
        %>                  End esp section
        -%>                 End esp section and trim trailing newline

        <%^ content         Mark the location to substitute content in a layout page
        <%^ global          Put esp code at the global level
        <%^ start           Put esp code at the start of the function
        <%^ end             Put esp code at the end of the function

        %!var               Substitue the value of a parameter.
        %$param             Substitue the value of a request parameter.
        %#field             Lookup the current record for the value of the field.
        %~                  Home URL for the application

    Deprecated:
        <%^ layout "file"   Specify a layout page to use. Use layout "" to disable layout management
        <%^ include "file"  Include an esp file
 */

//  DEPRECATED layout
PUBLIC char *espBuildScript(HttpRoute *route, cchar *page, cchar *path, cchar *cacheName, cchar *layout,
        EspState *state, char **err)
{
    EspState    top;
    EspParse    parse;
    MprBuf      *body;
    cchar       *layoutsDir;
    char        *control, *incText, *where, *layoutCode, *bodyCode;
    char        *rest, *include, *line, *fmt, *layoutPage, *incCode, *token;
    ssize       len;
    int         tid;

    assert(page);

    *err = 0;
    if (!state) {
        assert(cacheName);
        state = &top;
        memset(state, 0, sizeof(EspParse));
        state->global = mprCreateBuf(0, 0);
        state->start = mprCreateBuf(0, 0);
        state->end = mprCreateBuf(0, 0);
    }
    body = mprCreateBuf(0, 0);
    parse.data = (char*) page;
    parse.next = parse.data;
    parse.lineNumber = 0;
    parse.token = mprCreateBuf(0, 0);
    parse.path = path;
    tid = getEspToken(&parse);

    while (tid != ESP_TOK_EOF) {
        token = mprGetBufStart(parse.token);
#if KEEP
        if (state->lineNumber != lastLine) {
            mprPutToBuf(script, "\n# %d \"%s\"\n", state->lineNumber, path);
        }
#endif
        switch (tid) {
        case ESP_TOK_CODE:
#if DEPRECATED || 1
            if (*token == '^') {
                for (token++; *token && isspace((uchar) *token); token++) ;
                where = ssplit(token, " \t\r\n", &rest);
                if (*rest) {
                    if (smatch(where, "global")) {
                        mprPutStringToBuf(state->global, rest);

                    } else if (smatch(where, "start")) {
                        mprPutToBuf(state->start, "%s  ", rest);

                    } else if (smatch(where, "end")) {
                        mprPutToBuf(state->end, "%s  ", rest);
                    }
                }
            } else
#endif
            mprPutStringToBuf(body, fixMultiStrings(token));
            break;

        case ESP_TOK_CONTROL:
            control = ssplit(token, " \t\r\n", &token);
            if (smatch(control, "content")) {
                mprPutStringToBuf(body, ESP_CONTENT_MARKER);

#if DEPRECATED || 1
            } else if (smatch(control, "include")) {
                token = strim(token, " \t\r\n\"", MPR_TRIM_BOTH);
                token = mprNormalizePath(token);
                if (token[0] == '/') {
                    include = sclone(token);
                } else {
                    include = mprJoinPath(mprGetPathDir(path), token);
                }
                if ((incText = mprReadPathContents(include, &len)) == 0) {
                    *err = sfmt("Cannot read include file: %s", include);
                    return 0;
                }
                /* Recurse and process the include script */
                if ((incCode = espBuildScript(route, incText, include, NULL, NULL, state, err)) == 0) {
                    return 0;
                }
                mprPutStringToBuf(body, incCode);
#endif

#if DEPRECATED || 1
            } else if (smatch(control, "layout")) {
                mprLog("esp warn", 0, "Using deprecated \"layout\" control directive in esp page: %s", path);
                token = strim(token, " \t\r\n\"", MPR_TRIM_BOTH);
                if (*token == '\0') {
                    layout = 0;
                } else {
                    token = mprNormalizePath(token);
                    if (token[0] == '/') {
                        layout = sclone(token);
                    } else {
                        if ((layoutsDir = httpGetDir(route, "LAYOUTS")) != 0) {
                            layout = mprJoinPath(layoutsDir, token);
                        } else {
                            layout = mprJoinPath(mprGetPathDir(path), token);
                        }
                    }
                    if (!mprPathExists(layout, F_OK)) {
                        *err = sfmt("Cannot access layout page %s", layout);
                        return 0;
                    }
                }
#endif

            } else if (smatch(control, "global")) {
                mprPutStringToBuf(state->global, token);

            } else if (smatch(control, "start")) {
                mprPutToBuf(state->start, "%s  ", token);

            } else if (smatch(control, "end")) {
                mprPutToBuf(state->end, "%s  ", token);

            } else {
                *err = sfmt("Unknown control %s at line %d", control, state->lineNumber);
                return 0;
            }
            break;

        case ESP_TOK_ERR:
            return 0;

        case ESP_TOK_EXPR:
            /* <%= expr %> */
            if (*token == '%') {
                fmt = ssplit(token, ": \t\r\n", &token);
                /* Default without format is safe. If users want a format and safe, use %S or renderSafe() */
                token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
                mprPutToBuf(body, "  espRender(stream, \"%s\", %s);\n", fmt, token);
            } else {
                token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
                mprPutToBuf(body, "  espRenderSafeString(stream, %s);\n", token);
            }
            break;

        case ESP_TOK_VAR:
            /* %!var -- string variable */
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            mprPutToBuf(body, "  espRenderString(stream, %s);\n", token);
            break;

        case ESP_TOK_FIELD:
            /* %#field -- field in the current record */
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            mprPutToBuf(body, "  espRenderSafeString(stream, getField(getRec(), \"%s\"));\n", token);
            break;

        case ESP_TOK_PARAM:
            /* %$param -- variable in (param || session) - Safe render */
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            mprPutToBuf(body, "  espRenderVar(stream, \"%s\");\n", token);
            break;

        case ESP_TOK_HOME:
            /* %~ Home URL */
            if (parse.next[0] && parse.next[0] != '/' && parse.next[0] != '\'' && parse.next[0] != '"') {
                mprLog("esp warn", 0, "Using %%~ without following / in %s\n", path);
            }
            mprPutToBuf(body, "  espRenderString(stream, httpGetRouteTop(stream));");
            break;

#if DEPRECATED || 1
        //  DEPRECATED serverPrefix in version 6
        case ESP_TOK_SERVER:
            /* @| Server URL */
            mprLog("esp warn", 0, "Using deprecated \"|\" server URL directive in esp page: %s", path);
            mprPutToBuf(body, "  espRenderString(stream, sjoin(stream->rx->route->prefix ? stream->rx->route->prefix : \"\", stream->rx->route->serverPrefix, NULL));");
            break;
#endif

        case ESP_TOK_LITERAL:
            line = joinLine(token, &len);
            mprPutToBuf(body, "  espRenderBlock(stream, \"%s\", %zd);\n", line, len);
            break;

        default:
            return 0;
        }
        tid = getEspToken(&parse);
    }
    mprAddNullToBuf(body);

#if DEPRECATED || 1
    if (layout && mprPathExists(layout, R_OK)) {
        if ((layoutPage = mprReadPathContents(layout, &len)) == 0) {
            *err = sfmt("Cannot read layout page: %s", layout);
            return 0;
        }
        if ((layoutCode = espBuildScript(route, layoutPage, layout, NULL, NULL, state, err)) == 0) {
            return 0;
        }
#if ME_DEBUG
        if (!scontains(layoutCode, ESP_CONTENT_MARKER)) {
            *err = sfmt("Layout page is missing content marker: %s", layout);
            return 0;
        }
#endif
        bodyCode = sreplace(layoutCode, ESP_CONTENT_MARKER, mprGetBufStart(body));
        mprLog("esp warn", 0, "Using deprecated layouts in esp page: %s, use Expansive instead", path);
    } else
#endif
    bodyCode = mprGetBufStart(body);

    if (state == &top) {
        if (mprGetBufLength(state->start) > 0) {
            mprPutCharToBuf(state->start, '\n');
        }
        if (mprGetBufLength(state->end) > 0) {
            mprPutCharToBuf(state->end, '\n');
        }
        mprAddNullToBuf(state->global);
        mprAddNullToBuf(state->start);
        mprAddNullToBuf(state->end);
        bodyCode = sfmt(\
            "/*\n   Generated from %s\n */\n"\
            "#include \"esp.h\"\n"\
            "%s\n"\
            "static void %s(HttpStream *stream) {\n"\
            "%s%s%s"\
            "}\n\n"\
            "%s int esp_%s(HttpRoute *route) {\n"\
            "   espDefineView(route, \"%s\", %s);\n"\
            "   return 0;\n"\
            "}\n",
            mprGetRelPath(path, route->home), mprGetBufStart(state->global), cacheName,
                mprGetBufStart(state->start), bodyCode, mprGetBufStart(state->end),
            ESP_EXPORT_STRING, cacheName, mprGetPortablePath(mprGetRelPath(path, route->documents)), cacheName);
        mprDebug("esp", 5, "Create ESP script: \n%s\n", bodyCode);
    }
    return bodyCode;
}


static bool addChar(EspParse *parse, int c)
{
    if (mprPutCharToBuf(parse->token, c) < 0) {
        return 0;
    }
    mprAddNullToBuf(parse->token);
    return 1;
}


static char *eatSpace(EspParse *parse, char *next)
{
    for (; *next && isspace((uchar) *next); next++) {
        if (*next == '\n') {
            parse->lineNumber++;
        }
    }
    return next;
}


static char *eatNewLine(EspParse *parse, char *next)
{
    for (; *next && isspace((uchar) *next); next++) {
        if (*next == '\n') {
            parse->lineNumber++;
            next++;
            break;
        }
    }
    return next;
}


/*
    Get the next ESP input token. input points to the next input token.
    parse->token will hold the parsed token. The function returns the token id.
 */
static int getEspToken(EspParse *parse)
{
    char    *start, *end, *next;
    int     tid, done, c, t;

    start = next = parse->next;
    end = &start[slen(start)];
    mprFlushBuf(parse->token);
    tid = ESP_TOK_LITERAL;

    for (done = 0; !done && next < end; next++) {
        c = *next;
        switch (c) {
        case '<':
            if (next[1] == '%' && ((next == start) || next[-1] != '\\') && next[2] != '%') {
                next += 2;
                if (mprGetBufLength(parse->token) > 0) {
                    next -= 3;
                } else {
                    next = eatSpace(parse, next);
                    if (*next == '=') {
                        /*
                            <%= directive
                         */
                        tid = ESP_TOK_EXPR;
                        next = eatSpace(parse, ++next);
                        while (next < end && !(*next == '%' && next[1] == '>' && (next[-1] != '\\' && next[-1] != '%'))) {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }

                    //  DEPRECATED '@'
                    } else if (*next == '@' || *next == '^') {
                        /*
                            <%^ control
                         */
                        if (*next == '@') {
                            mprLog("esp warn", 0, "Using deprecated \"%%%c\" control directive in esp page: %s",
                                *next, parse->path);
                        }
                        tid = ESP_TOK_CONTROL;
                        next = eatSpace(parse, ++next);
                        while (next < end && !(*next == '%' && next[1] == '>' && (next[-1] != '\\' && next[-1] != '%'))) {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }

                    } else {
                        tid = ESP_TOK_CODE;
                        while (next < end && !(*next == '%' && next[1] == '>' && (next[-1] != '\\' && next[-1] != '%'))) {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }
                    }
                    if (*next && next > start && next[-1] == '-') {
                        /* Remove "-" */
                        mprAdjustBufEnd(parse->token, -1);
                        mprAddNullToBuf(parse->token);
                        next = eatNewLine(parse, next + 2) - 1;
                    } else {
                        next++;
                    }
                }
                done++;
            } else {
                if (!addChar(parse, c)) {
                    return ESP_TOK_ERR;
                }
            }
            break;

        case '%':
            if (next > start && (next[-1] == '\\' || next[-1] == '%')) {
                break;
            }
            if ((next == start) || next[-1] != '\\') {
                t = next[1];
                if (t == '~') {
                    next += 2;
                    if (mprGetBufLength(parse->token) > 0) {
                        next -= 3;
                    } else {
                        tid = ESP_TOK_HOME;
                        if (!addChar(parse, c) || !addChar(parse, t)) {
                            return ESP_TOK_ERR;
                        }
                        next--;
                    }
                    done++;

#if DEPRECATED || 1
                } else if (t == '|') {
                    mprLog("esp warn", 0, "CC Using deprecated \"|\" control directive in esp page: %s", parse->path);
                    next += 2;
                    if (mprGetBufLength(parse->token) > 0) {
                        next -= 3;
                    } else {
                        tid = ESP_TOK_SERVER;
                        if (!addChar(parse, c)) {
                            return ESP_TOK_ERR;
                        }
                        next--;
                    }
                    done++;
#endif

                //  DEPRECATED '@'
                } else if (t == '!' || t == '@' || t == '#' || t == '$') {
                    next += 2;
                    if (mprGetBufLength(parse->token) > 0) {
                        next -= 3;
                    } else {
                        if (t == '!') {
                           tid = ESP_TOK_VAR;
#if DEPRECATED || 1
                        } else if (t == '@') {
                            tid = ESP_TOK_PARAM;
#endif
                        } else if (t == '#') {
                            tid = ESP_TOK_FIELD;
                        } else {
                            tid = ESP_TOK_PARAM;
                        }
                        next = eatSpace(parse, next);
                        while (isalnum((uchar) *next) || *next == '_') {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }
                        next--;
                    }
                    done++;
                } else {
                    if (!addChar(parse, c)) {
                        return ESP_TOK_ERR;
                    }
                    done++;
                }
            }
            break;

        case '\n':
            parse->lineNumber++;
            /* Fall through */

        case '\r':
        default:
            if (c == '\"' || c == '\\') {
                if (!addChar(parse, '\\')) {
                    return ESP_TOK_ERR;
                }
            }
            if (!addChar(parse, c)) {
                return ESP_TOK_ERR;
            }
            break;
        }
    }
    if (mprGetBufLength(parse->token) == 0) {
        tid = ESP_TOK_EOF;
    }
    parse->next = next;
    return tid;
}


static cchar *getEnvString(HttpRoute *route, cchar *key, cchar *defaultValue)
{
    EspRoute    *eroute;
    cchar       *value;

    eroute = route->eroute;
    if (route->config) {
        if ((value = mprGetJson(route->config, sfmt("esp.app.tokens.%s", key))) != 0) {
            return value;
        }
    }
    if (!eroute || !eroute->env || (value = mprLookupKey(eroute->env, key)) == 0) {
        if ((value = getenv(key)) == 0) {
            if (defaultValue) {
                value = defaultValue;
            } else {
                value = sfmt("${%s}", key);
            }
        }
    }
    return value;
}


static cchar *getShobjExt(cchar *os)
{
    if (smatch(os, "macosx")) {
        return ".dylib";
    } else if (smatch(os, "windows")) {
        return ".dll";
    } else if (smatch(os, "vxworks")) {
        return ".out";
    } else {
        return ".so";
    }
}


static cchar *getShlibExt(cchar *os)
{
    if (smatch(os, "macosx")) {
        return ".dylib";
    } else if (smatch(os, "windows")) {
        return ".lib";
    } else if (smatch(os, "vxworks")) {
        return ".a";
    } else {
        return ".so";
    }
}


static cchar *getObjExt(cchar *os)
{
    if (smatch(os, "windows")) {
        return ".obj";
    }
    return ".o";
}


static cchar *getArExt(cchar *os)
{
    if (smatch(os, "windows")) {
        return ".lib";
    }
    return ".a";
}


static cchar *getCompilerName(cchar *os, cchar *arch)
{
    cchar       *name;

    name = "gcc";
    if (smatch(os, "vxworks")) {
        if (smatch(arch, "x86") || smatch(arch, "i586") || smatch(arch, "i686") || smatch(arch, "pentium")) {
            name = "ccpentium";
        } else if (scontains(arch, "86")) {
            name = "cc386";
        } else if (scontains(arch, "ppc")) {
            name = "ccppc";
        } else if (scontains(arch, "xscale") || scontains(arch, "arm")) {
            name = "ccarm";
        } else if (scontains(arch, "68")) {
            name = "cc68k";
        } else if (scontains(arch, "sh")) {
            name = "ccsh";
        } else if (scontains(arch, "mips")) {
            name = "ccmips";
        }
    } else if (smatch(os, "macosx")) {
        name = "clang";
    }
    return name;
}


static cchar *getVxCPU(cchar *arch)
{
    char   *cpu, *family;

    family = ssplit(sclone(arch), ":", &cpu);
    if (*cpu == '\0') {
        if (smatch(family, "i386")) {
            cpu = "I80386";
        } else if (smatch(family, "i486")) {
            cpu = "I80486";
        } else if (smatch(family, "x86") || sends(family, "86")) {
            cpu = "PENTIUM";
        } else if (scaselessmatch(family, "mips")) {
            cpu = "MIPS32";
        } else if (scaselessmatch(family, "arm")) {
            cpu = "ARM7TDMI";
        } else if (scaselessmatch(family, "ppc")) {
            cpu = "PPC";
        } else {
            cpu = (char*) arch;
        }
    }
    return supper(cpu);
}


static cchar *getDebug(EspRoute *eroute)
{
    Http        *http;
    Esp         *esp;
    cchar       *switches;
    int         symbols;

    http = MPR->httpService;
    esp = MPR->espService;
    symbols = 0;
    if (esp->compileMode == ESP_COMPILE_SYMBOLS) {
        symbols = 1;
    } else if (esp->compileMode == ESP_COMPILE_OPTIMIZED) {
        symbols = 0;
    } else if (eroute->compileMode == ESP_COMPILE_SYMBOLS) {
        symbols = 1;
    } else if (eroute->compileMode == ESP_COMPILE_OPTIMIZED) {
        symbols = 0;
    } else {
        symbols = sends(http->platform, "-debug") || sends(http->platform, "-xcode") ||
            sends(http->platform, "-mine") || sends(http->platform, "-vsdebug");
    }
    if (scontains(http->platform, "windows-")) {
        switches = (symbols) ? "-Zi -Od" : "-Os";
    } else {
        switches = (symbols) ? "-g" : "-O2";
    }
    return sfmt("%s%s", switches, eroute->combine ? " -DESP_COMBINE=1" : "");
}


static cchar *getLibs(cchar *os)
{
    cchar       *libs;

    if (smatch(os, "windows")) {
        libs = "\"${LIBPATH}\\libesp${SHLIB}\" \"${LIBPATH}\\libhttp.lib\" \"${LIBPATH}\\libmpr.lib\"";
    } else {
#if LINUX
        /*
            Fedora interprets $ORIGN relative to the shared library and not the application executable
            So loading compiled apps fails to locate libesp.so.
            Since building a shared library, can omit libs and resolve at load time.
         */
        libs = "";
#else
        libs = "-lesp -lpcre -lhttp -lmpr -lpthread -lm";
#endif
    }
    return libs;
}


static bool matchToken(cchar **str, cchar *token)
{
    ssize   len;

    len = slen(token);
    if (sncmp(*str, token, len) == 0) {
        *str += len;
        return 1;
    }
    return 0;
}


static cchar *getMappedArch(cchar *arch)
{
    if (smatch(arch, "x64")) {
        arch = "x86_64";
    } else if (smatch(arch, "x86")) {
        arch = "i686";
    }
    return arch;
}


#if WINDOWS
static int reverseSortVersions(char **s1, char **s2)
{
    return -scmp(*s1, *s2);
}
#endif


#if ME_WIN_LIKE
static cchar *getWinSDK(HttpRoute *route)
{
    EspRoute *eroute;

    /*
        MS has made a huge mess of where and how the windows SDKs are installed. The registry key at
        HKLM/Software/Microsoft/Microsoft SDKs/Windows/CurrentInstallFolder cannot be trusted and often
        points to the old 7.X SDKs even when 8.X is installed and active. MS have also moved the 8.X
        SDK to Windows Kits, while still using the old folder for some bits. So the old-reliable
        CurrentInstallFolder registry key is now unusable. So we must scan for explicit SDK versions
        listed above. Ugh!
     */
    cchar   *path, *key, *version;
    MprList *versions;
    int     i;

    eroute = route->eroute;
    if (eroute->winsdk) {
        return eroute->winsdk;
    }
    /*
        General strategy is to find an "include" directory in the highest version Windows SDK.
        First search the registry key: Windows Kits/InstalledRoots/KitsRoot*
     */
    key = sfmt("HKLM\\SOFTWARE%s\\Microsoft\\Windows Kits\\Installed Roots", (ME_64) ? "\\Wow6432Node" : "");
    versions = mprListRegistry(key);
    mprSortList(versions, (MprSortProc) reverseSortVersions, 0);
    path = 0;
    for (ITERATE_ITEMS(versions, version, i)) {
        if (scontains(version, "KitsRoot")) {
            path = mprReadRegistry(key, version);
            if (mprPathExists(mprJoinPath(path, "Include"), X_OK)) {
                break;
            }
            path = 0;
        }
    }
    if (!path) {
        /*
            Next search the registry keys at Windows SDKs/Windows/ * /InstallationFolder
         */
        key = sfmt("HKLM\\SOFTWARE%s\\Microsoft\\Microsoft SDKs\\Windows", (ME_64) ? "\\Wow6432Node" : "");
        versions = mprListRegistry(key);
        mprSortList(versions, (MprSortProc) reverseSortVersions, 0);
        for (ITERATE_ITEMS(versions, version, i)) {
            if ((path = mprReadRegistry(sfmt("%s\\%s", key, version), "InstallationFolder")) != 0) {
                if (mprPathExists(mprJoinPath(path, "Include"), X_OK)) {
                    break;
                }
                path = 0;
            }
        }
    }
    if (!path) {
        /* Last chance: Old Windows SDK 7 registry location */
        path = mprReadRegistry("HKLM\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows", "CurrentInstallFolder");
    }
    if (!path) {
        path = "${WINSDK}";
    }
    mprLog("info esp", 5, "Using Windows SDK at %s", path);
    eroute->winsdk = strim(path, "\\", MPR_TRIM_END);
    return eroute->winsdk;
}


static cchar *getWinVer(HttpRoute *route)
{
    MprList     *versions;
    cchar       *winver, *winsdk;

    winsdk = getWinSDK(route);
    versions = mprGlobPathFiles(mprJoinPath(winsdk, "Lib"), "*", MPR_PATH_RELATIVE);
    mprSortList(versions, 0, 0);
    if ((winver = mprGetLastItem(versions)) == 0) {
        winver = sclone("win8");
    }
    return winver;
}
#endif


static cchar *getArPath(cchar *os, cchar *arch)
{
#if WINDOWS
    /*
        Get the real system architecture (32 or 64 bit)
     */
    Http *http = MPR->httpService;
    cchar *path = espGetVisualStudio();
    if (getenv("VSINSTALLDIR")) {
        path = sclone("lib.exe");
    } else if (scontains(http->platform, "-x64-")) {
        int is64BitSystem = smatch(getenv("PROCESSOR_ARCHITECTURE"), "AMD64") || getenv("PROCESSOR_ARCHITEW6432");
        if (is64BitSystem) {
            path = mprJoinPath(path, "VC/bin/amd64/lib.exe");
        } else {
            /* Cross building on a 32-bit system */
            path = mprJoinPath(path, "VC/bin/x86_amd64/lib.exe");
        }
    } else {
        path = mprJoinPath(path, "VC/bin/lib.exe");
    }
    return path;
#else
    return "ar";
#endif
}


static cchar *getCompilerPath(cchar *os, cchar *arch)
{
#if WINDOWS
    /*
        Get the real system architecture (32 or 64 bit)
     */
    Http *http = MPR->httpService;
    cchar *path = espGetVisualStudio();
    if (getenv("VSINSTALLDIR")) {
        path = sclone("cl.exe");
    } else if (scontains(http->platform, "-x64-")) {
        int is64BitSystem = smatch(getenv("PROCESSOR_ARCHITECTURE"), "AMD64") || getenv("PROCESSOR_ARCHITEW6432");
        if (is64BitSystem) {
            path = mprJoinPath(path, "VC/bin/amd64/cl.exe");
        } else {
            /* Cross building on a 32-bit system */
            path = mprJoinPath(path, "VC/bin/x86_amd64/cl.exe");
        }
    } else {
        path = mprJoinPath(path, "VC/bin/cl.exe");
    }
    return path;
#else
    return getCompilerName(os, arch);
#endif
}


static CompileContext* allocContext(cchar *source, cchar *csource, cchar *module, cchar *cache)
{
    CompileContext *context;

    if ((context = mprAllocObj(CompileContext, manageContext)) == 0) {
        return 0;
    }
    /*
        Use actual references to ensure we retain the memory
     */
    context->csource = csource;
    context->source = source;
    context->module = module;
    context->cache = cache;
    return context;
}


static void manageContext(CompileContext *context, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(context->csource);
        mprMark(context->source);
        mprMark(context->module);
        mprMark(context->cache);
    }
}

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    mdb.c -- ESP In-Memory Embedded Database (MDB)

    WARNING: This is prototype code

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "http.h"


#include    "pcre.h"

#if ME_COM_MDB
/************************************* Local **********************************/

#define MDB_LOAD_BEGIN   1      /* Initial state */
#define MDB_LOAD_TABLE   2      /* Parsing a table */
#define MDB_LOAD_HINTS   3      /* Parsing hints */
#define MDB_LOAD_SCHEMA  4      /* Parsing schema */
#define MDB_LOAD_COL     5      /* Parsing column schema */
#define MDB_LOAD_DATA    6      /* Parsing data */
#define MDB_LOAD_FIELD   7      /* Parsing fields */

/*
    Operations for mdbFindGrid
 */
#define OP_ERR      -1          /* Illegal operation */
#define OP_EQ       0           /* "==" Equal operation */
#define OP_NEQ      0x2         /* "!=" Not equal operation */
#define OP_LT       0x4         /* "<" Less than operation */
#define OP_GT       0x8         /* ">" Greater than operation */
#define OP_LTE      0x10        /* ">=" Less than or equal operation */
#define OP_GTE      0x20        /* "<=" Greater than or equal operation */
#define OP_IN       0x40        /* Contains */

/************************************ Forwards ********************************/

static void autoSave(Mdb *mdb, MdbTable *table);
static MdbCol *createCol(MdbTable *table, cchar *columnName);
static EdiRec *createRecFromRow(Edi *edi, MdbRow *row);
static MdbRow *createRow(Mdb *mdb, MdbTable *table);
static MdbCol *getCol(MdbTable *table, int col);
static MdbRow *getRow(MdbTable *table, int rid);
static MdbTable *getTable(Mdb *mdb, int tid);
static MdbSchema *growSchema(MdbTable *table);
static MdbCol *lookupField(MdbTable *table, cchar *columnName);
static int lookupRow(MdbTable *table, cchar *key);
static MdbTable *lookupTable(Mdb *mdb, cchar *tableName);
static void manageCol(MdbCol *col, int flags);
static void manageMdb(Mdb *mdb, int flags);
static void manageRow(MdbRow *row, int flags);
static void manageSchema(MdbSchema *schema, int flags);
static void manageTable(MdbTable *table, int flags);
static int parseOperation(cchar *operation);
static int updateFieldValue(MdbRow *row, MdbCol *col, cchar *value);
static int mdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static int mdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
static int mdbAddTable(Edi *edi, cchar *tableName);
static int mdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static void mdbClose(Edi *edi);
static EdiRec *mdbCreateRec(Edi *edi, cchar *tableName);
static int mdbDelete(cchar *path);
static EdiGrid *mdbFindGrid(Edi *edi, cchar *tableName, cchar *query);
static MprList *mdbGetColumns(Edi *edi, cchar *tableName);
static int mdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid);
static MprList *mdbGetTables(Edi *edi);
static int mdbGetTableDimensions(Edi *edi, cchar *tableName, int *numRows, int *numCols);
static int mdbLoad(Edi *edi, cchar *path);
static int mdbLoadFromString(Edi *edi, cchar *string);
static int mdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName);
static Edi *mdbOpen(cchar *path, int flags);
static EdiGrid *mdbQuery(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list vargs);
static EdiField mdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
static EdiRec *mdbReadRecByKey(Edi *edi, cchar *tableName, cchar *key);
static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);
static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);
static int mdbRemoveRec(Edi *edi, cchar *tableName, cchar *key);
static int mdbRemoveTable(Edi *edi, cchar *tableName);
static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);
static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
static int mdbSave(Edi *edi);
static int mdbUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
static int mdbUpdateRec(Edi *edi, EdiRec *rec);

static EdiProvider MdbProvider = {
    "mdb",
    mdbAddColumn, mdbAddIndex, mdbAddTable, mdbChangeColumn, mdbClose, mdbCreateRec, mdbDelete,
    mdbGetColumns, mdbGetColumnSchema, mdbGetTables, mdbGetTableDimensions, mdbLoad, mdbLookupField, mdbOpen, mdbQuery,
    mdbReadField, mdbFindGrid, mdbReadRecByKey, mdbRemoveColumn, mdbRemoveIndex, mdbRemoveRec, mdbRemoveTable,
    mdbRenameTable, mdbRenameColumn, mdbSave, mdbUpdateField, mdbUpdateRec,
};

/************************************* Code ***********************************/

PUBLIC void mdbInit(void)
{
    ediAddProvider(&MdbProvider);
}


static Mdb *mdbAlloc(cchar *path, int flags)
{
    Mdb      *mdb;

    if ((mdb = mprAllocObj(Mdb, manageMdb)) == 0) {
        return 0;
    }
    mdb->edi.provider = &MdbProvider;
    mdb->edi.flags = flags;
    mdb->edi.path = sclone(path);
    mdb->edi.schemaCache = mprCreateHash(0, 0);
    mdb->edi.mutex = mprCreateLock();
    mdb->edi.validations = mprCreateHash(0, 0);
    return mdb;
}


static void manageMdb(Mdb *mdb, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mdb->edi.path);
        mprMark(mdb->edi.schemaCache);
        mprMark(mdb->edi.errMsg);
        mprMark(mdb->edi.validations);
        mprMark(mdb->edi.mutex);
        mprMark(mdb->tables);
        /* Don't mark load fields */
    } else {
        mdbClose((Edi*) mdb);
    }
}


static void mdbClose(Edi *edi)
{
    Mdb     *mdb;

    mdb = (Mdb*) edi;
    if (mdb->tables) {
        autoSave(mdb, 0);
        mdb->tables = 0;
    }
}


/*
    Create a record based on the table's schema. Not saved to the database.
 */
static EdiRec *mdbCreateRec(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;
    EdiRec      *rec;
    EdiField    *fp;
    int         f, nfields;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        mprLog("error esp mdb", 0, "Cannot find table %s", tableName);
        return 0;
    }
    nfields = max(table->schema->ncols, 1);
    if ((rec = mprAllocBlock(sizeof(EdiRec) + sizeof(EdiField) * nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(rec, (MprManager) ediManageEdiRec);

    rec->edi = edi;
    rec->tableName = table->name;
    rec->nfields = nfields;

    for (f = 0; f < nfields; f++) {
        col = getCol(table, f);
        fp = &rec->fields[f];
        fp->type = col->type;
        fp->name = col->name;
        fp->flags = col->flags;
    }
    return rec;
}


static int mdbDelete(cchar *path)
{
    return mprDeletePath(path);
}


static Edi *mdbOpen(cchar *source, int flags)
{
    Mdb     *mdb;

    if (flags & EDI_LITERAL) {
        flags |= EDI_NO_SAVE;
        if ((mdb = mdbAlloc("literal", flags)) == 0) {
            return 0;
        }
        if (mdbLoadFromString((Edi*) mdb, source) < 0) {
            return 0;
        }
    } else {
        if ((mdb = mdbAlloc(source, flags)) == 0) {
            return 0;
        }
        if (!mprPathExists(source, R_OK)) {
            if (flags & EDI_CREATE) {
                mdbSave((Edi*) mdb);
            } else {
                return 0;
            }
        }
        if (mdbLoad((Edi*) mdb, source) < 0) {
            return 0;
        }
    }
    return (Edi*) mdb;
}


static int mdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    assert(edi);
    assert(tableName && *tableName);
    assert(columnName && *columnName);
    assert(type);

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, columnName)) != 0) {
        unlock(edi);
        return MPR_ERR_ALREADY_EXISTS;
    }
    if ((col = createCol(table, columnName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    col->type = type;
    col->flags = flags;
    if (flags & EDI_INDEX) {
        if (table->index) {
            mprLog("warn esp mdb", 0, "Index already specified in table %s, replacing.", tableName);
        }
        if ((table->index = mprCreateHash(0, MPR_HASH_STATIC_VALUES | MPR_HASH_STABLE)) != 0) {
            table->indexCol = col;
        }
    }
    autoSave(mdb, table);
    unlock(edi);
    return 0;

}


/*
    IndexName is not implemented yet
 */
static int mdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    assert(edi);
    assert(tableName && *tableName);
    assert(columnName && *columnName);

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, columnName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((table->index = mprCreateHash(0, MPR_HASH_STATIC_VALUES | MPR_HASH_STABLE)) == 0) {
        unlock(edi);
        return MPR_ERR_MEMORY;
    }
    table->indexCol = col;
    col->flags |= EDI_INDEX;
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


static int mdbAddTable(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;

    assert(edi);
    assert(tableName && *tableName);

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) != 0) {
        unlock(edi);
        return MPR_ERR_ALREADY_EXISTS;
    }
    if ((table = mprAllocObj(MdbTable, manageTable)) == 0) {
        unlock(edi);
        return MPR_ERR_MEMORY;
    }
    if ((table->rows = mprCreateList(0, MPR_LIST_STABLE)) == 0) {
        unlock(edi);
        return MPR_ERR_MEMORY;
    }
    table->name = sclone(tableName);
    if (mdb->tables == 0) {
        mdb->tables = mprCreateList(0, MPR_LIST_STABLE);
    }
    if (!growSchema(table)) {
        unlock(edi);
        return MPR_ERR_MEMORY;
    }
    mprAddItem(mdb->tables, table);
    autoSave(mdb, lookupTable(mdb, tableName));
    unlock(edi);
    return 0;
}


static int mdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    assert(edi);
    assert(tableName && *tableName);
    assert(columnName && *columnName);
    assert(type);

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, columnName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    col->name = sclone(columnName);
    col->type = type;
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


/*
    Return a list of column names
 */
static MprList *mdbGetColumns(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbSchema   *schema;
    MprList     *list;
    int         i;

    assert(edi);
    assert(tableName && *tableName);

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return 0;
    }
    schema = table->schema;
    assert(schema);
    list = mprCreateList(schema->ncols, MPR_LIST_STABLE);
    for (i = 0; i < schema->ncols; i++) {
        /* No need to clone */
        mprAddItem(list, schema->cols[i].name);
    }
    unlock(edi);
    return list;
}


/*
    Make a field. WARNING: the value is not cloned
 */
static EdiField makeFieldFromRow(MdbRow *row, MdbCol *col)
{
    EdiField    f;

    /* Note: the value is not cloned */
    f.value = row->fields[col->cid];
    f.type = col->type;
    f.name = col->name;
    f.flags = col->flags;
    f.valid = 1;
    return f;
}


static int mdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    if (type) {
        *type = -1;
    }
    if (cid) {
        *cid = -1;
    }
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, columnName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if (type) {
        *type = col->type;
    }
    if (flags) {
        *flags = col->flags;
    }
    if (cid) {
        *cid = col->cid;
    }
    unlock(edi);
    return 0;
}


static MprList *mdbGetTables(Edi *edi)
{
    Mdb         *mdb;
    MprList     *list;
    MdbTable     *table;
    int         tid, ntables;

    mdb = (Mdb*) edi;
    lock(edi);
    list = mprCreateList(-1, MPR_LIST_STABLE);
    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = mprGetItem(mdb->tables, tid);
        mprAddItem(list, table->name);
    }
    unlock(edi);
    return list;
}


static int mdbGetTableDimensions(Edi *edi, cchar *tableName, int *numRows, int *numCols)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    lock(edi);
    if (numRows) {
        *numRows = 0;
    }
    if (numCols) {
        *numCols = 0;
    }
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if (numRows) {
        *numRows = mprGetListLength(table->rows);
    }
    if (numCols) {
        *numCols = table->schema->ncols;
    }
    unlock(edi);
    return 0;
}


static int mdbLoad(Edi *edi, cchar *path)
{
    cchar       *data;
    ssize       len;

    if ((data = mprReadPathContents(path, &len)) == 0) {
        return MPR_ERR_CANT_READ;
    }
    return mdbLoadFromString(edi, data);
}


static int mdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, fieldName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    unlock(edi);
    return col->cid;
}


static EdiGrid *mdbQuery(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list vargs)
{
    mprLog("error esp mdb", 0, "MDB does not implement ediQuery");
    return 0;
}


static EdiField mdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;
    MdbRow      *row;
    EdiField    field, err;
    int         r;

    mdb = (Mdb*) edi;
    lock(edi);
    err.valid = 0;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return err;
    }
    if ((col = lookupField(table, fieldName)) == 0) {
        unlock(edi);
        return err;
    }
    if ((r = lookupRow(table, key)) < 0) {
        unlock(edi);
        return err;
    }
    row = mprGetItem(table->rows, r);
    field = makeFieldFromRow(row, col);
    unlock(edi);
    return field;
}


static EdiRec *mdbReadRecByKey(Edi *edi, cchar *tableName, cchar *key)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    EdiRec      *rec;
    int         r;

    mdb = (Mdb*) edi;
    rec = 0;

    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return 0;
    }
    if ((r = lookupRow(table, key)) < 0) {
        unlock(edi);
        return 0;
    }
    if ((row = mprGetItem(table->rows, r)) != 0) {
        rec = createRecFromRow(edi, row);
    }
    unlock(edi);
    return rec;
}


static bool matchCell(MdbCol *col, cchar *existing, int op, cchar *value)
{
    if (value == 0 || *value == '\0') {
        return 0;
    }
    switch (op) {
    case OP_IN:
        if (scontains(slower(existing), slower(value))) {
            return 1;
        }
        break;
    case OP_EQ:
        if (smatch(existing, value)) {
            return 1;
        }
        break;
    case OP_NEQ:
        if (!smatch(existing, value)) {
            return 1;
        }
        break;
    case OP_LT:
        if (scmp(existing, value) < 0) {
            return 1;
        }
        break;
    case OP_GT:
        if (scmp(existing, value) > 0) {
            return 1;
        }
        break;
    case OP_LTE:
        if (scmp(existing, value) <= 0) {
            return 1;
        }
        break;
    case OP_GTE:
        if (scmp(existing, value) >= 0) {
            return 1;
        }
        break;
    default:
        assert(0);
    }
    return 0;
}


static bool matchRow(MdbTable *table, MdbRow *row, int op, cchar *value)
{
    MdbCol      *col;
    MdbSchema   *schema;

    schema = table->schema;
    for (col = schema->cols; col < &schema->cols[schema->ncols]; col++) {
        if (matchCell(col, row->fields[col->cid], op, value)) {
            return 1;
        }
    }
    return 0;
}


/*
    parse a SQL like query expression.
 */
static MprList *parseMdbQuery(cchar *query, int *offsetp, int *limitp)
{
    MprList     *expressions;
    char        *cp, *limit, *offset, *tok;

    *offsetp = *limitp = 0;
    expressions = mprCreateList(0, 0);
    query = sclone(query);
    if ((cp = scaselesscontains(query, "LIMIT ")) != 0) {
        *cp = '\0';
        cp += 6;
        offset = stok(cp, ", ", &limit);
        if (!offset || !limit) {
            return 0;
        }
        *offsetp = (int) stoi(offset);
        *limitp = (int) stoi(limit);
    }
    query = strim(query, " ", 0);
    for (tok = sclone(query); *tok && (cp = scontains(tok, " AND ")) != 0; ) {
        *cp = '\0';
        cp += 5;
        mprAddItem(expressions, tok);
        tok = cp;
    }
    if (tok && *tok) {
        mprAddItem(expressions, tok);
    }
    return expressions;
}


static EdiGrid *mdbFindGrid(Edi *edi, cchar *tableName, cchar *query)
{
    Mdb         *mdb;
    EdiGrid     *grid;
    MdbTable    *table;
    MdbCol      *col;
    MdbRow      *row;
    MprList     *expressions;
    cchar       *columnName, *expression, *operation;
    char        *tok, *value;
    int         limit, matched, nrows, next, offset, op, r, index, count, nextExpression;

    assert(edi);
    assert(tableName && *tableName);
    columnName = operation = value = 0;
    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return 0;
    }
    nrows = mprGetListLength(table->rows);
    if ((grid = ediCreateBareGrid(edi, tableName, nrows)) == 0) {
        unlock(edi);
        return 0;
    }
    grid->flags = EDI_GRID_READ_ONLY;
    grid->nrecords = 0;
    grid->count = table->rows->length;

    if ((expressions = parseMdbQuery(query, &offset, &limit)) == 0) {
        unlock(edi);
        return 0;
    }
    if (limit <= 0) {
        limit = MAXINT;
    }
    if (offset < 0) {
        offset = 0;
    }
    count = index = 0;

    /*
        Optimized path for solo indexed lookup on "id"
     */
    if (mprGetListLength(expressions) == 1) {
        expression = mprGetItem(expressions, 0);
        columnName = stok(sclone(expression), " ", &tok);
        operation = stok(tok, " ", &value);
        if (smatch(columnName, "id") && smatch(operation, "==")) {
            if ((col = lookupField(table, columnName)) == 0) {
                unlock(edi);
                return 0;
            }
            if (col->flags & EDI_INDEX) {
                if ((r = lookupRow(table, value)) >= 0) {
                    row = getRow(table, r);
                    grid->records[0] = createRecFromRow(edi, row);
                    grid->nrecords = 1;
                }
                unlock(edi);
                return grid;
            }
        }
    }

    /*
        Linear search
     */
    for (ITERATE_ITEMS(table->rows, row, next)) {
        matched = 1;
        for (ITERATE_ITEMS(expressions, expression, nextExpression)) {
            columnName = stok(sclone(expression), " ", &tok);
            operation = stok(tok, " ", &value);
            if ((op = parseOperation(operation)) < 0) {
                unlock(edi);
                return 0;
            }
            if (smatch(columnName, "*")) {
                if (!matchRow(table, row, op, value)) {
                    matched = 0;
                    break;
                }
            } else {
                if ((col = lookupField(table, columnName)) == 0) {
                    unlock(edi);
                    return 0;
                }
                if (!matchCell(col, row->fields[col->cid], op, value)) {
                    matched = 0;
                    break;
                }
            }
        }
        if (matched && count++ >= offset) {
            grid->records[index++] = createRecFromRow(edi, row);
            grid->nrecords = index;
            if (--limit <= 0) {
                break;
            }
        }
    }
    unlock(edi);
    return grid;
}


static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbSchema   *schema;
    MdbCol      *col;
    int         c;

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, columnName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if (table->indexCol == col) {
        table->index = 0;
        table->indexCol = 0;
    }
    if (table->keyCol == col) {
        table->keyCol = 0;
    }
    schema = table->schema;
    assert(schema);
    for (c = col->cid; c < schema->ncols; c++) {
        schema->cols[c] = schema->cols[c + 1];
    }
    schema->ncols--;
    schema->cols[schema->ncols].name = 0;
    assert(schema->ncols >= 0);
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    table->index = 0;
    if (table->indexCol) {
        table->indexCol->flags &= ~EDI_INDEX;
        table->indexCol = 0;
        autoSave(mdb, table);
    }
    unlock(edi);
    return 0;
}


static int mdbRemoveRec(Edi *edi, cchar *tableName, cchar *key)
{
    Mdb         *mdb;
    MdbTable    *table;
    MprKey      *kp;
    int         r, rc, value;

    assert(edi);
    assert(tableName && *tableName);
    assert(key && *key);

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((r = lookupRow(table, key)) < 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    rc = mprRemoveItemAtPos(table->rows, r);
    if (table->index) {
        mprRemoveKey(table->index, key);
        for (ITERATE_KEYS(table->index, kp)) {
            value = (int) PTOL(kp->data);
            if (value >= r) {
                mprAddKey(table->index, kp->key, LTOP(value - 1));
            }
        }
    }
    autoSave(mdb, table);
    unlock(edi);
    return rc;
}


static int mdbRemoveTable(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    int         next;

    mdb = (Mdb*) edi;
    lock(edi);
    for (ITERATE_ITEMS(mdb->tables, table, next)) {
        if (smatch(table->name, tableName)) {
            mprRemoveItem(mdb->tables, table);
            autoSave(mdb, table);
            unlock(edi);
            return 0;
        }
    }
    unlock(edi);
    return MPR_ERR_CANT_FIND;
}


static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    table->name = sclone(newTableName);
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, columnName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    col->name = sclone(newColumnName);
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


static int mdbUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    int         r;

    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupField(table, fieldName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if ((r = lookupRow(table, key)) < 0) {
        row = createRow(mdb, table);
    }
    if ((row = getRow(table, r)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    updateFieldValue(row, col, value);
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


static int mdbUpdateRec(Edi *edi, EdiRec *rec)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    int         f, r;

    if (!ediValidateRec(rec)) {
        return MPR_ERR_CANT_WRITE;
    }
    mdb = (Mdb*) edi;
    lock(edi);
    if ((table = lookupTable(mdb, rec->tableName)) == 0) {
        unlock(edi);
        return MPR_ERR_CANT_FIND;
    }
    if (rec->id == 0 || (r = lookupRow(table, rec->id)) < 0) {
        row = createRow(mdb, table);
    } else {
        if ((row = getRow(table, r)) == 0) {
            unlock(edi);
            return MPR_ERR_CANT_FIND;
        }
    }
    for (f = 0; f < rec->nfields; f++) {
        if ((col = getCol(table, f)) != 0) {
            updateFieldValue(row, col, rec->fields[f].value);
        }
    }
    autoSave(mdb, table);
    unlock(edi);
    return 0;
}


/******************************** Database Loading ***************************/

static void clearLoadState(Mdb *mdb)
{
    mdb->loadNcols = 0;
    mdb->loadCol = 0;
    mdb->loadRow = 0;
}


static void pushState(Mdb *mdb, int state)
{
    mprPushItem(mdb->loadStack, LTOP(state));
    mdb->loadState = state;
}


static void popState(Mdb *mdb)
{
    mprPopItem(mdb->loadStack);
    mdb->loadState = (int) PTOL(mprGetLastItem(mdb->loadStack));
    assert(mdb->loadState > 0);
}


static int checkMdbState(MprJsonParser *jp, cchar *name, bool leave)
{
    Mdb     *mdb;

    mdb = jp->data;
    if (leave) {
        popState(mdb);
        return 0;
    }
    switch (mdb->loadState) {
    case MDB_LOAD_BEGIN:
        if (mdbAddTable((Edi*) mdb, name) < 0) {
            return MPR_ERR_MEMORY;
        }
        mdb->loadTable = lookupTable(mdb, name);
        clearLoadState(mdb);
        pushState(mdb, MDB_LOAD_TABLE);
        break;

    case MDB_LOAD_TABLE:
        if (smatch(name, "hints")) {
            pushState(mdb, MDB_LOAD_HINTS);
        } else if (smatch(name, "schema")) {
            pushState(mdb, MDB_LOAD_SCHEMA);
        } else if (smatch(name, "data")) {
            pushState(mdb, MDB_LOAD_DATA);
        } else {
            mprSetJsonError(jp, "Bad property '%s'", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_SCHEMA:
        if ((mdb->loadCol = createCol(mdb->loadTable, name)) == 0) {
            mprSetJsonError(jp, "Cannot create '%s' column", name);
            return MPR_ERR_MEMORY;
        }
        pushState(mdb, MDB_LOAD_COL);
        break;

    case MDB_LOAD_DATA:
        if ((mdb->loadRow = createRow(mdb, mdb->loadTable)) == 0) {
            return MPR_ERR_MEMORY;
        }
        mdb->loadCid = 0;
        pushState(mdb, MDB_LOAD_FIELD);
        break;

    case MDB_LOAD_HINTS:
    case MDB_LOAD_COL:
    case MDB_LOAD_FIELD:
        pushState(mdb, mdb->loadState);
        break;

    default:
        mprSetJsonError(jp, "Potential corrupt data. Bad state");
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static int setMdbValue(MprJsonParser *parser, MprJson *obj, cchar *name, MprJson *child)
{
    Mdb         *mdb;
    MdbCol      *col;
    cchar       *value;

    mdb = parser->data;
    value = child->value;

    switch (mdb->loadState) {
    case MDB_LOAD_BEGIN:
    case MDB_LOAD_TABLE:
    case MDB_LOAD_SCHEMA:
    case MDB_LOAD_DATA:
        break;

    case MDB_LOAD_HINTS:
        if (smatch(name, "ncols")) {
            mdb->loadNcols = atoi(value);
        } else {
            mprSetJsonError(parser, "Unknown hint '%s'", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_COL:
        if (smatch(name, "index")) {
            mdbAddIndex((Edi*) mdb, mdb->loadTable->name, mdb->loadCol->name, NULL);
        } else if (smatch(name, "type")) {
            if ((mdb->loadCol->type = ediParseTypeString(value)) <= 0) {
                mprSetJsonError(parser, "Bad column type %s", value);
                return MPR_ERR_BAD_FORMAT;
            }
        } else if (smatch(name, "key")) {
            mdb->loadCol->flags |= EDI_KEY;
            mdb->loadTable->keyCol = mdb->loadCol;
        } else if (smatch(name, "autoinc")) {
            mdb->loadCol->flags |= EDI_AUTO_INC;
        } else if (smatch(name, "foreign")) {
            mdb->loadCol->flags |= EDI_FOREIGN;
        } else if (smatch(name, "notnull")) {
            mdb->loadCol->flags |= EDI_NOT_NULL;
        } else {
            mprSetJsonError(parser, "Bad property '%s' in column definition", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_FIELD:
        if ((col = getCol(mdb->loadTable, mdb->loadCid++)) == 0) {
            mprSetJsonError(parser, "Bad state '%d' in setMdbValue, column %s,  potential corrupt data", mdb->loadState, name);
            return MPR_ERR_BAD_FORMAT;
        }
        updateFieldValue(mdb->loadRow, col, value);
        break;

    default:
        mprSetJsonError(parser, "Bad state '%d' in setMdbValue potential corrupt data", mdb->loadState);
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static int mdbLoadFromString(Edi *edi, cchar *str)
{
    Mdb             *mdb;
    MprJson         *obj;
    MprJsonCallback cb;
    cchar           *errorMsg;

    mdb = (Mdb*) edi;
    mdb->edi.flags |= EDI_SUPPRESS_SAVE;
    mdb->edi.flags |= MDB_LOADING;
    mdb->loadStack = mprCreateList(0, MPR_LIST_STABLE);
    pushState(mdb, MDB_LOAD_BEGIN);

    memset(&cb, 0, sizeof(cb));
    cb.checkBlock = checkMdbState;
    cb.setValue = setMdbValue;

    obj = mprParseJsonEx(str, &cb, mdb, 0, &errorMsg);
    mdb->edi.flags &= ~MDB_LOADING;
    mdb->loadStack = 0;
    if (obj == 0) {
        mprError("Cannot load database %s", errorMsg);
        return MPR_ERR_CANT_LOAD;
    }
    mdb->edi.flags &= ~EDI_SUPPRESS_SAVE;
    return 0;
}


/******************************** Database Saving ****************************/

static void autoSave(Mdb *mdb, MdbTable *table)
{
    assert(mdb);

    if (mdb->edi.flags & EDI_NO_SAVE) {
        return;
    }
    if (mdb->edi.flags & EDI_AUTO_SAVE && !(mdb->edi.flags & EDI_SUPPRESS_SAVE)) {
        if (mdbSave((Edi*) mdb) < 0) {
            mprLog("error esp mdb", 0, "Cannot save database %s", mdb->edi.path);
        }
    }
}


static int mdbSave(Edi *edi)
{
    Mdb         *mdb;
    MdbSchema   *schema;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    cchar       *value, *path, *cp;
    char        *npath, *bak, *type;
    MprFile     *out;
    int         cid, rid, tid, ntables, nrows;

    mdb = (Mdb*) edi;
    if (mdb->edi.flags & EDI_NO_SAVE) {
        return MPR_ERR_BAD_STATE;
    }
    path = mdb->edi.path;
    if (path == 0) {
        mprLog("error esp mdb", 0, "Cannot save MDB database in mdbSave, no path specified");
        return MPR_ERR_BAD_ARGS;
    }
    npath = mprReplacePathExt(path, "new");
    if ((out = mprOpenFile(npath, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
        mprLog("error esp mdb", 0, "Cannot open database %s", npath);
        return 0;
    }
    mprEnableFileBuffering(out, 0, 0);
    mprWriteFileFmt(out, "{\n");

    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = getTable(mdb, tid);
        schema = table->schema;
        assert(schema);
        mprWriteFileFmt(out, "    '%s': {\n", table->name);
        mprWriteFileFmt(out, "        hints: {\n            ncols: %d\n        },\n", schema->ncols);
        mprWriteFileString(out, "        schema: {\n");
        /* Skip the id which is always the first column */
        for (cid = 0; cid < schema->ncols; cid++) {
            col = getCol(table, cid);
            type = ediGetTypeString(col->type);
            mprWriteFileFmt(out, "            '%s': { type: '%s'", col->name, type);
            if (col->flags & EDI_AUTO_INC) {
                mprWriteFileString(out, ", autoinc: true");
            }
            if (col->flags & EDI_INDEX) {
                mprWriteFileString(out, ", index: true");
            }
            if (col->flags & EDI_KEY) {
                mprWriteFileString(out, ", key: true");
            }
            if (col->flags & EDI_FOREIGN) {
                mprWriteFileString(out, ", foreign: true");
            }
            if (col->flags & EDI_NOT_NULL) {
                mprWriteFileString(out, ", notnull: true");
            }
            mprWriteFileString(out, " },\n");
        }
        mprWriteFileString(out, "        },\n");
        mprWriteFileString(out, "        data: [\n");

        nrows = mprGetListLength(table->rows);
        for (rid = 0; rid < nrows; rid++) {
            mprWriteFileString(out, "            [ ");
            row = getRow(table, rid);
            for (cid = 0; cid < schema->ncols; cid++) {
                col = getCol(table, cid);
                value = row->fields[col->cid];
                if (value == 0 && col->flags & EDI_AUTO_INC) {
                    row->fields[col->cid] = itos(++col->lastValue);
                }
                if (value == 0) {
                    mprWriteFileFmt(out, "null, ");
                } else if (col->type == EDI_TYPE_STRING || col->type == EDI_TYPE_TEXT) {
                    mprWriteFile(out, "'", 1);
                    /*
                        The MPR JSON parser is tolerant of embedded, unquoted control characters. So only need
                        to worry about embedded single quotes and back quote.
                     */
                    for (cp = value; *cp; cp++) {
                        if (*cp == '\'' || *cp == '\\') {
                            mprWriteFile(out, "\\", 1);
                        }
                        mprWriteFile(out, cp, 1);
                    }
                    mprWriteFile(out, "',", 2);
                } else {
                    for (cp = value; *cp; cp++) {
                        if (*cp == '\'' || *cp == '\\') {
                            mprWriteFile(out, "\\", 1);
                        }
                        mprWriteFile(out, cp, 1);
                    }
                    mprWriteFile(out, ",", 1);
                }
            }
            mprWriteFileString(out, "],\n");
        }
        mprWriteFileString(out, "        ],\n    },\n");
    }
    mprWriteFileString(out, "}\n");
    mprCloseFile(out);

    bak = mprReplacePathExt(path, "bak");
    mprDeletePath(bak);
    if (mprPathExists(path, R_OK) && rename(path, bak) < 0) {
        mprLog("error esp mdb", 0, "Cannot rename %s to %s", path, bak);
        return MPR_ERR_CANT_WRITE;
    }
    if (rename(npath, path) < 0) {
        mprLog("error esp mdb", 0, "Cannot rename %s to %s", npath, path);
        /* Restore backup */
        rename(bak, path);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


/********************************* Table Operations **************************/

static MdbTable *getTable(Mdb *mdb, int tid)
{
    int         ntables;

    ntables = mprGetListLength(mdb->tables);
    if (tid < 0 || tid >= ntables) {
        return 0;
    }
    return mprGetItem(mdb->tables, tid);
}


static MdbTable *lookupTable(Mdb *mdb, cchar *tableName)
{
    MdbTable     *table;
    int         tid, ntables;

    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = mprGetItem(mdb->tables, tid);
        if (smatch(table->name, tableName)) {
            return table;
        }
    }
    return 0;
}


static void manageTable(MdbTable *table, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(table->name);
        mprMark(table->schema);
        mprMark(table->index);
        mprMark(table->rows);
        /*
            Do not mark keyCol or indexCol - they are unmanaged
         */
    }
}


static int lookupRow(MdbTable *table, cchar *key)
{
    MprKey      *kp;
    MdbRow      *row;
    int         nrows, r, keycol;

    if (table->index) {
        if ((kp = mprLookupKeyEntry(table->index, key)) != 0) {
            return (int) PTOL(kp->data);
        }
    } else {
        nrows = mprGetListLength(table->rows);
        keycol = table->keyCol ? table->keyCol->cid : 0;
        for (r = 0; r < nrows; r++) {
            row = mprGetItem(table->rows, r);
            if (smatch(row->fields[keycol], key)) {
                return r;
            }
        }
    }
    return -1;
}


/********************************* Schema / Col ****************************/

static MdbSchema *growSchema(MdbTable *table)
{
    if (table->schema == 0) {
        if ((table->schema = mprAllocBlock(sizeof(MdbSchema) +
                sizeof(MdbCol) * MDB_INCR, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
            return 0;
        }
        mprSetManager(table->schema, (MprManager) manageSchema);
        table->schema->capacity = MDB_INCR;

    } else if (table->schema->ncols >= table->schema->capacity) {
        if ((table->schema = mprRealloc(table->schema, sizeof(MdbSchema) +
                (sizeof(MdbCol) * (table->schema->capacity + MDB_INCR)))) == 0) {
            return 0;
        }
        memset(&table->schema->cols[table->schema->capacity], 0, MDB_INCR * sizeof(MdbCol));
        table->schema->capacity += MDB_INCR;
    }
    return table->schema;
}


static MdbCol *createCol(MdbTable *table, cchar *columnName)
{
    MdbSchema    *schema;
    MdbCol       *col;

    if ((col = lookupField(table, columnName)) != 0) {
        return 0;
    }
    if ((schema = growSchema(table)) == 0) {
        return 0;
    }
    col = &schema->cols[schema->ncols];
    col->cid = schema->ncols++;
    col->name = sclone(columnName);
    return col;
}


static void manageSchema(MdbSchema *schema, int flags)
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        for (i = 0; i < schema->ncols; i++) {
            manageCol(&schema->cols[i], flags);
        }
    }
}


static void manageCol(MdbCol *col, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(col->name);
    }
}


static MdbCol *getCol(MdbTable *table, int col)
{
    if (col < 0 || col >= table->schema->ncols) {
        return 0;
    }
    return &table->schema->cols[col];
}


static MdbCol *lookupField(MdbTable *table, cchar *columnName)
{
    MdbSchema    *schema;
    MdbCol       *col;

    if ((schema = growSchema(table)) == 0) {
        return 0;
    }
    for (col = schema->cols; col < &schema->cols[schema->ncols]; col++) {
        if (smatch(col->name, columnName)) {
            return col;
        }
    }
    return 0;
}


/********************************* Row Operations **************************/

static MdbRow *createRow(Mdb *mdb, MdbTable *table)
{
    MdbRow      *row;
    int         ncols;

    ncols = max(table->schema->ncols, 1);
    if ((row = mprAllocBlock(sizeof(MdbRow) + sizeof(EdiField) * ncols, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(row, (MprManager) manageRow);
    row->table = table;
    row->nfields = ncols;
    row->rid = mprAddItem(table->rows, row);
    return row;
}


static void manageRow(MdbRow *row, int flags)
{
    int     fid;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(row->table);
        for (fid = 0; fid < row->nfields; fid++) {
            mprMark(row->fields[fid]);
        }
    }
}


static MdbRow *getRow(MdbTable *table, int rid)
{
    int     nrows;

    nrows = mprGetListLength(table->rows);
    if (rid < 0 || rid > nrows) {
        return 0;
    }
    return mprGetItem(table->rows, rid);
}

/********************************* Field Operations ************************/

static cchar *mapMdbValue(cchar *value, int type)
{
    MprTime     time;

    if (value == 0) {
        return value;
    }
    switch (type) {
    case EDI_TYPE_DATE:
        if (!snumber(value)) {
            mprParseTime(&time, value, MPR_UTC_TIMEZONE, 0);
            value = itos(time);
        }
        break;

    case EDI_TYPE_BOOL:
        if (smatch(value, "false")) {
            value = "0";
        } else if (smatch(value, "true")) {
            value = "1";
        }
        break;

    case EDI_TYPE_BINARY:
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
    default:
        break;
    }
    return sclone(value);
}

static int updateFieldValue(MdbRow *row, MdbCol *col, cchar *value)
{
    MdbTable    *table;
    cchar       *key;

    assert(row);
    assert(col);

    table = row->table;
    if (col->flags & EDI_INDEX) {
        if ((key = row->fields[col->cid]) != 0) {
            mprRemoveKey(table->index, key);
        }
    } else {
        key = 0;
    }
    if (col->flags & EDI_AUTO_INC) {
        if (value == 0) {
            row->fields[col->cid] = value = itos(++col->lastValue);
        } else {
            row->fields[col->cid] = sclone(value);
            col->lastValue = max(col->lastValue, (int64) stoi(value));
        }
    } else {
        row->fields[col->cid] = mapMdbValue(value, col->type);
    }
    if (col->flags & EDI_INDEX && value) {
        mprAddKey(table->index, value, LTOP(row->rid));
    }
    return 0;
}

/*********************************** Support *******************************/
/*
    Optimized record creation
 */
static EdiRec *createRecFromRow(Edi *edi, MdbRow *row)
{
    EdiRec  *rec;
    MdbCol  *col;
    int     c;

    if ((rec = mprAllocBlock(sizeof(EdiRec) + sizeof(EdiField) * row->nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(rec, (MprManager) ediManageEdiRec);
    rec->edi = edi;
    rec->tableName = row->table->name;
    rec->id = row->fields[0];
    rec->nfields = row->nfields;
    for (c = 0; c < row->nfields; c++) {
        col = getCol(row->table, c);
        rec->fields[c] = makeFieldFromRow(row, col);
    }
    return rec;
}


static int parseOperation(cchar *operation)
{
    if (!operation) {
        return OP_EQ;
    }
    switch (*operation) {
    case '=':
        if (smatch(operation, "==")) {
            return OP_EQ;
        }
        break;
    case '!':
        if (smatch(operation, "=!")) {
            return OP_EQ;
        }
        break;
    case '<':
        if (smatch(operation, "<")) {
            return OP_LT;
        } else if (smatch(operation, "<=")) {
            return OP_LTE;
        }
        break;
    case '>':
        if (smatch(operation, "><")) {
            return OP_IN;
        } else if (smatch(operation, ">")) {
            return OP_GT;
        } else if (smatch(operation, ">=")) {
            return OP_GTE;
        }
    }
    mprLog("error esp mdb", 0, "Unknown read operation '%s'", operation);
    return OP_ERR;
}


#else
/* To prevent ar/ranlib warnings */
PUBLIC void mdbDummy() {}

#endif /* ME_COM_MDB */

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

/*
    sdb.c -- ESP SQLite Database (SDB)

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "http.h"


#if ME_COM_SQLITE
 #include    "sqlite3.h"

#ifndef ME_MAX_SQLITE_MEM
    #define ME_MAX_SQLITE_MEM      (2*1024*1024)   /**< Maximum buffering for Sqlite */
#endif
#ifndef ME_MAX_SQLITE_DURATION
    #define ME_MAX_SQLITE_DURATION 30000           /**< Database busy timeout */
#endif

/************************************* Local **********************************/

typedef struct Sdb {
    Edi             edi;            /**< EDI database interface structure */
    sqlite3         *db;            /**< SQLite database handle */
    MprHash         *schemas;       /**< Table schemas */
} Sdb;

static int sqliteInitialized;
static void initSqlite(void);

#if KEEP
static char *DataTypeToSqlType[] = {
    "binary":       "blob",
    "boolean":      "tinyint",
    "date":         "date",
    "datetime":     "datetime",
    "decimal":      "decimal",
    "float":        "float",
    "integer":      "int",
    "number":       "decimal",
    "string":       "varchar",
    "text":         "text",
    "time":         "time",
    "timestamp":    "datetime",
    0, 0,
};
#endif

static char *dataTypeToSqlType[] = {
                            0,
    /* EDI_TYPE_BINARY */   "BLOB",
    /* EDI_TYPE_BOOL */     "TINYINT",      //  "INTEGER",
    /* EDI_TYPE_DATE */     "DATE",         //  "TEXT",
    /* EDI_TYPE_FLOAT */    "FLOAT",        //  "REAL",
    /* EDI_TYPE_INT */      "INTEGER",
    /* EDI_TYPE_STRING */   "STRING",       //  "TEXT",
    /* EDI_TYPE_TEXT */     "TEXT",
                            0,
};

/************************************ Forwards ********************************/

static EdiRec *createBareRec(Edi *edi, cchar *tableName, int nfields);
static EdiField makeRecField(cchar *value, cchar *name, int type);
static void manageSdb(Sdb *sdb, int flags);
static int mapSqliteTypeToEdiType(int type);
static cchar *mapToSqlType(int type);
static int mapToEdiType(cchar *type);
static EdiGrid *query(Edi *edi, cchar *cmd, ...);
static EdiGrid *queryArgv(Edi *edi, cchar *cmd, int argc, cchar **argv, ...);
static EdiGrid *queryv(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list args);
static int sdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static int sdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
static int sdbAddTable(Edi *edi, cchar *tableName);
static int sdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static void sdbClose(Edi *edi);
static EdiRec *sdbCreateRec(Edi *edi, cchar *tableName);
static int sdbDelete(cchar *path);
static void sdbError(Edi *edi, cchar *fmt, ...);
static int sdbRemoveRec(Edi *edi, cchar *tableName, cchar *key);
static EdiGrid *sdbFindGrid(Edi *edi, cchar *tableName, cchar *select);
static MprList *sdbGetColumns(Edi *edi, cchar *tableName);
static int sdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid);
static MprList *sdbGetTables(Edi *edi);
static int sdbGetTableDimensions(Edi *edi, cchar *tableName, int *numRows, int *numCols);
static int sdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName);
static Edi *sdbOpen(cchar *path, int flags);
PUBLIC EdiGrid *sdbQuery(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list vargs);
static EdiField sdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
static EdiRec *sdbReadRecByKey(Edi *edi, cchar *tableName, cchar *key);
static int sdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);
static int sdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);
static int sdbRemoveTable(Edi *edi, cchar *tableName);
static int sdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);
static int sdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
static int sdbSave(Edi *edi);
static void sdbDebug(Edi *edi, int level, cchar *fmt, ...);
static int sdbUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
static int sdbUpdateRec(Edi *edi, EdiRec *rec);
static bool validName(cchar *str);

static EdiProvider SdbProvider = {
    "sdb",
    sdbAddColumn, sdbAddIndex, sdbAddTable, sdbChangeColumn, sdbClose, sdbCreateRec, sdbDelete,
    sdbGetColumns, sdbGetColumnSchema, sdbGetTables, sdbGetTableDimensions, NULL, sdbLookupField, sdbOpen, sdbQuery,
    sdbReadField, sdbFindGrid, sdbReadRecByKey, sdbRemoveColumn, sdbRemoveIndex, sdbRemoveRec, sdbRemoveTable,
    sdbRenameTable, sdbRenameColumn, sdbSave, sdbUpdateField, sdbUpdateRec,
};

/************************************* Code ***********************************/

PUBLIC void sdbInit(void)
{
    ediAddProvider(&SdbProvider);
}


static Sdb *sdbCreate(cchar *path, int flags)
{
    Sdb      *sdb;

    assert(path && *path);

    initSqlite();
    if ((sdb = mprAllocObj(Sdb, manageSdb)) == 0) {
        return 0;
    }
    sdb->edi.flags = flags;
    sdb->edi.provider = &SdbProvider;
    sdb->edi.path = sclone(path);
    sdb->edi.schemaCache = mprCreateHash(0, 0);
    sdb->edi.validations = mprCreateHash(0, 0);
    sdb->edi.mutex = mprCreateLock();
    sdb->schemas = mprCreateHash(0, MPR_HASH_STABLE);
    return sdb;
}


static void manageSdb(Sdb *sdb, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sdb->edi.path);
        mprMark(sdb->edi.schemaCache);
        mprMark(sdb->edi.errMsg);
        mprMark(sdb->edi.mutex);
        mprMark(sdb->edi.validations);
        mprMark(sdb->schemas);

    } else if (flags & MPR_MANAGE_FREE) {
        sdbClose((Edi*) sdb);
    }
}


static void sdbClose(Edi *edi)
{
    Sdb     *sdb;

    assert(edi);

    sdb = (Sdb*) edi;
    if (sdb->db) {
        sqlite3_close(sdb->db);
        sdb->db = 0;
    }
}


static void removeSchema(Edi *edi, cchar *tableName)
{
    mprRemoveKey(((Sdb*) edi)->schemas, tableName);
}


static EdiRec *getSchema(Edi *edi, cchar *tableName)
{
    Sdb         *sdb;
    EdiRec      *rec, *schema;
    EdiField    *fp;
    EdiGrid     *grid;
    int         r;

    sdb = (Sdb*) edi;
    if (!validName(tableName)) {
        return 0;
    }
    if ((schema = mprLookupKey(sdb->schemas, tableName)) != 0) {
        return schema;
    }
    /*
        Each result row represents an EDI column: CID, Name, Type, NotNull, Dflt_value, PK
     */
    if ((grid = query(edi, sfmt("PRAGMA table_info(%s);", tableName), NULL)) == 0) {
        return 0;
    }
    schema = createBareRec(edi, tableName, grid->nrecords);
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        fp = &schema->fields[r];
        fp->name = rec->fields[1].value;
        fp->type = mapToEdiType(rec->fields[2].value);
        if (rec->fields[5].value && rec->fields[5].value[0] == '1') {
            fp->flags = EDI_KEY;
        }
    }
    mprAddKey(sdb->schemas, tableName, schema);
    return schema;
}


/*
    Create a record based on the table's schema. Not saved to the database.
 */
static EdiRec *sdbCreateRec(Edi *edi, cchar *tableName)
{
    EdiRec  *rec, *schema;
    int     i;

    schema = getSchema(edi, tableName);
    if ((rec = mprAllocBlock(sizeof(EdiRec) + sizeof(EdiField) * schema->nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(rec, (MprManager) ediManageEdiRec);
    rec->edi = edi;
    rec->tableName = sclone(tableName);
    rec->nfields = schema->nfields;
    for (i = 0; i < schema->nfields; i++) {
        rec->fields[i].name = schema->fields[i].name;
        rec->fields[i].type = schema->fields[i].type;
        rec->fields[i].flags = schema->fields[i].flags;
    }
    return rec;
}


static int sdbDelete(cchar *path)
{
    assert(path && *path);
    return mprDeletePath(path);
}


static Edi *sdbOpen(cchar *path, int flags)
{
    Sdb     *sdb;

    assert(path && *path);
    if ((sdb = sdbCreate(path, flags)) == 0) {
        return 0;
    }
    if (mprPathExists(path, R_OK) || (flags & EDI_CREATE)) {
        if (sqlite3_open(path, &sdb->db) != SQLITE_OK) {
            mprLog("error esp sdb", 0, "Cannot open database %s", path);
            return 0;
        }
        sqlite3_soft_heap_limit(ME_MAX_SQLITE_MEM);
        sqlite3_busy_timeout(sdb->db, ME_MAX_SQLITE_DURATION);
    } else {
        return 0;
    }
    return (Edi*) sdb;
}


static int sdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    assert(edi);
    assert(tableName && *tableName);
    assert(columnName && *columnName);
    assert(type > 0);

    if (!validName(tableName) || !validName(columnName)) {
        return MPR_ERR_BAD_ARGS;
    }
    if (sdbLookupField(edi, tableName, columnName) >= 0) {
        /* Already exists */
        return 0;
    }
    removeSchema(edi, tableName);
    /*
        The field types are used for the SQLite column affinity settings
     */
    if (query(edi, sfmt("ALTER TABLE %s ADD %s %s", tableName, columnName, mapToSqlType(type)), NULL) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int sdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName)
{
    assert(edi);
    assert(tableName && *tableName);
    assert(columnName && *columnName);
    assert(indexName && *indexName);

    if (!validName(tableName) || !validName(columnName) || !validName(indexName)) {
        return MPR_ERR_BAD_ARGS;
    }
    return query(edi, sfmt("CREATE INDEX %s ON %s (%s);", indexName, tableName, columnName), NULL) != 0;
}


static int sdbAddTable(Edi *edi, cchar *tableName)
{
    assert(edi);
    assert(tableName && *tableName);

    if (!validName(tableName)) {
        return MPR_ERR_BAD_ARGS;
    }
    if (query(edi, sfmt("DROP TABLE IF EXISTS %s;", tableName), NULL) == 0) {
        return MPR_ERR_CANT_DELETE;
    }
    /*
        SQLite cannot add a primary key after the table is created
     */
    removeSchema(edi, tableName);
    return query(edi, sfmt("CREATE TABLE %s (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);", tableName), NULL) != 0;
}


static int sdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    mprLog("error esp sdb", 0, "SDB does not support changing columns");
    return MPR_ERR_BAD_STATE;
}


static MprList *sdbGetColumns(Edi *edi, cchar *tableName)
{
    MprList     *result;
    EdiRec      *schema;
    int         c;

    assert(edi);
    assert(tableName && *tableName);

    if ((schema = getSchema(edi, tableName)) == 0) {
        return 0;
    }
    if ((result = mprCreateList(0, MPR_LIST_STABLE)) == 0) {
        return 0;
    }
    for (c = 0; c < schema->nfields; c++) {
        mprAddItem(result, schema->fields[c].name);
    }
    return result;
}



static int sdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid)
{
    EdiRec      *schema;
    EdiField    *fp;
    int         c;

    assert(edi);
    assert(tableName && *tableName);
    assert(columnName && *columnName);

    schema = getSchema(edi, tableName);
    for (c = 0; c < schema->nfields; c++) {
        fp = &schema->fields[c];
        if (smatch(columnName, fp->name)) {
            if (type) {
                *type = fp->type;
            }
            if (flags) {
                *flags = fp->flags;
            }
            if (cid) {
                *cid = c;
            }
        }
    }
    return 0;
}


static MprList *sdbGetTables(Edi *edi)
{
    EdiGrid     *grid;
    EdiRec      *rec;
    MprList     *result;
    int         r;

    assert(edi);

    if ((grid = query(edi, "SELECT name from sqlite_master WHERE type = 'table' order by NAME;", NULL)) == 0) {
        return 0;
    }
    if ((result = mprCreateList(0, MPR_LIST_STABLE)) == 0) {
        return 0;
    }
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        if (sstarts(rec->tableName, "sqlite_")) {
            continue;
        }
        mprAddItem(result, rec->tableName);
    }
    return result;
}


static int sdbGetTableDimensions(Edi *edi, cchar *tableName, int *numRows, int *numCols)
{
    EdiGrid     *grid;
    EdiRec      *schema;

    assert(edi);
    assert(tableName && *tableName);

    if (numRows) {
        *numRows = 0;
    }
    if (numCols) {
        *numCols = 0;
    }
    if (!validName(tableName)) {
        return MPR_ERR_BAD_ARGS;
    }
    if (numRows) {
        if ((grid = query(edi, sfmt("SELECT COUNT(*) FROM %s;", tableName), NULL)) == 0) {
            return MPR_ERR_BAD_STATE;
        }
        if (grid->nrecords && grid->records[0]->fields->value) {
            *numRows = (int) stoi(grid->records[0]->fields->value);
        }
    }
    if (numCols) {
        if ((schema = getSchema(edi, tableName)) == 0) {
            return MPR_ERR_CANT_FIND;
        }
        *numCols = schema->nfields;
    }
    return 0;
}


static int sdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName)
{
    EdiRec      *schema;
    int         c;

    assert(edi);
    assert(tableName && *tableName);
    assert(fieldName && *fieldName);

    if ((schema = getSchema(edi, tableName)) == 0) {
        return 0;
    }
    for (c = 0; c < schema->nfields; c++) {
        /* Name is second field */
        if (smatch(fieldName, schema->fields[c].name)) {
            return c;
        }
    }
    return MPR_ERR_CANT_FIND;
}


PUBLIC EdiGrid *sdbQuery(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list vargs)
{
    return queryv(edi, cmd, argc, argv, vargs);
}


static EdiField sdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    EdiField    err;
    EdiGrid     *grid;

    if (!validName(tableName) || !validName(fieldName)) {
        err.valid = 0;
        return err;
    }
    if ((grid = query(edi, sfmt("SELECT %s FROM %s WHERE 'id' = ?;", fieldName, tableName), key, NULL)) == 0) {
        err.valid = 0;
        return err;
    }
    if (grid->nrecords == 0) {
        err.valid = 0;
        return err;
    }
    if (grid->records[0]->nfields == 0) {
        err.valid = 0;
        return err;
    }
    return grid->records[0]->fields[0];
}


static EdiRec *sdbReadRecByKey(Edi *edi, cchar *tableName, cchar *key)
{
    EdiGrid     *grid;

    if (!validName(tableName)) {
        return 0;
    }
    if ((grid = query(edi, sfmt("SELECT * FROM %s WHERE id = ?;", tableName), key, NULL)) == 0) {
        return 0;
    }
    if (grid->nrecords == 0) {
        return 0;
    }
    return grid->records[0];
}


static EdiGrid *setTableName(EdiGrid *grid, cchar *tableName)
{
    if (grid && !grid->tableName) {
        grid->tableName = sclone(tableName);
    }
    return grid;
}


/*
    parse a SQL like query expression.
 */
static cchar *parseSdbQuery(cchar *query, int *offsetp, int *limitp)
{
    MprList     *expressions;
    char        *cp, *limit, *offset, *tok;

    *offsetp = *limitp = 0;
    expressions = mprCreateList(0, 0);
    query = sclone(query);
    if ((cp = scaselesscontains(query, "LIMIT ")) != 0) {
        *cp = '\0';
        cp += 6;
        offset = stok(cp, ", ", &limit);
        if (!offset || !limit) {
            return 0;
        }
        *offsetp = (int) stoi(offset);
        *limitp = (int) stoi(limit);
    }
    query = strim(query, " ", 0);
    for (tok = sclone(query); *tok && (cp = scontains(tok, " AND ")) != 0; ) {
        *cp = '\0';
        cp += 5;
        mprAddItem(expressions, tok);
        tok = cp;
    }
    if (tok && *tok) {
        mprAddItem(expressions, tok);
    }
    return mprListToString(expressions, " ");
}


static EdiGrid *sdbFindGrid(Edi *edi, cchar *tableName, cchar *select)
{
    EdiGrid     *grid;
    EdiRec      *schema;
    MprBuf      *buf;
    cchar       *columnName, *expressions, *operation, *sql, **values;
    char        *tok, *value;
    int         i, limit, offset;

    assert(tableName && *tableName);
    columnName = operation = value = 0;

    if (select) {
        if ((expressions = parseSdbQuery(select, &offset, &limit)) == 0) {
            return 0;
        }
        /*
            Only works for a single query term
         */
        columnName = stok(sclone(expressions), " ", &tok);
        operation = stok(tok, " ", &value);
        if (smatch(operation, "><")) {
            operation = "LIKE";
            value = sfmt("%%%s%%", value);
        }
    }
    if (!validName(tableName)) {
        return 0;
    }
    if (limit <= 0) {
        limit = MAXINT;
    }
    if (offset < 0) {
        offset = 0;
    }
    if (columnName) {
        if (smatch(columnName, "*")) {
            schema = getSchema(edi, tableName);
            if (!schema) {
                return 0;
            }
            values = mprAllocZeroed(sizeof(cchar*) * (schema->nfields + 1));
            buf = mprCreateBuf(0, 0);
            if (!values || !buf) {
                return 0;
            }
            mprPutToBuf(buf, "SELECT * FROM %s WHERE ", tableName);
            for (i = 0; i < schema->nfields; i++) {
                mprPutToBuf(buf, "(%s %s ?)", schema->fields[i].name, operation);
                if ((i+1) < schema->nfields) {
                    mprPutStringToBuf(buf, " OR ");
                }
                values[i] = value;
            }
            mprPutToBuf(buf, " LIMIT %d, %d;", offset, limit);
            sql = mprBufToString(buf);
            grid = queryArgv(edi, sql, schema->nfields, values, NULL);

        } else {
            if (!validName(columnName)) {
                return 0;
            }
            sql = sfmt("SELECT * FROM %s WHERE %s %s ? LIMIT %d, %d;", tableName, columnName, operation, offset, limit);
            grid = query(edi, sql, value, NULL);
        }
    } else {
        sql = sfmt("SELECT * FROM %s LIMIT %d, %d;", tableName, offset, limit);
        grid = query(edi, sql, NULL);
    }
    if (grid) {
        if (offset > 0 || grid->nrecords == limit) {
            sdbGetTableDimensions(edi, tableName, &grid->count, NULL);
        } else {
            grid->count = grid->nrecords;
        }
    }
    return setTableName(grid, tableName);
}


static int sdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName)
{
    mprLog("error esp sdb", 0, "SDB does not support removing columns");
    return MPR_ERR_BAD_STATE;
}


static int sdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName)
{
    if (!validName(tableName) || !validName(indexName)) {
        return 0;
    }
    return query(edi, sfmt("DROP INDEX %s;", indexName), NULL) != 0;
}


static int sdbRemoveRec(Edi *edi, cchar *tableName, cchar *key)
{
    assert(edi);
    assert(tableName && *tableName);
    assert(key && *key);

    if (!validName(tableName)) {
        return 0;
    }
    return query(edi, sfmt("DELETE FROM %s WHERE id = ?;", tableName), key, NULL) != 0;
}


static int sdbRemoveTable(Edi *edi, cchar *tableName)
{
    if (!validName(tableName)) {
        return 0;
    }
    return query(edi, sfmt("DROP TABLE IF EXISTS %s;", tableName), NULL) != 0;
}


static int sdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName)
{
    if (!validName(tableName) || !validName(newTableName)) {
        return 0;
    }
    removeSchema(edi, tableName);
    removeSchema(edi, newTableName);
    return query(edi, sfmt("ALTER TABLE %s RENAME TO %s;", tableName, newTableName), NULL) != 0;
}


static int sdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName)
{
    mprLog("error esp sdb", 0, "SQLite does not support renaming columns");
    return MPR_ERR_BAD_STATE;
}


static int sdbSave(Edi *edi)
{
    return 0;
}


/*
    Map values before storing in the database
    While not required, it is prefereable to normalize the storage of some values.
    For example: dates are stored as numbers
 */
static cchar *mapSdbValue(cchar *value, int type)
{
    MprTime     time;

    if (value == 0) {
        return value;
    }
    switch (type) {
    case EDI_TYPE_DATE:
        if (!snumber(value)) {
            mprParseTime(&time, value, MPR_UTC_TIMEZONE, 0);
            value = itos(time);
        }
        break;

    case EDI_TYPE_BINARY:
    case EDI_TYPE_BOOL:
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
    default:
        break;
    }
    return value;
}


static int sdbUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    int     type;

    if (!validName(tableName) || !validName(fieldName)) {
        return 0;
    }
    sdbGetColumnSchema(edi, tableName, fieldName, &type, 0, 0);
    value = mapSdbValue(value, type);
    return query(edi, sfmt("UPDATE %s SET %s TO ? WHERE 'id' = ?;", tableName, fieldName), value, key, NULL) != 0;
}


/*
    Use parameterized queries to reduce the risk of SQL injection
 */
static int sdbUpdateRec(Edi *edi, EdiRec *rec)
{
    MprBuf      *buf;
    EdiField    *fp;
    cchar       **argv;
    int         argc, f;

    if (!ediValidateRec(rec)) {
        return MPR_ERR_CANT_WRITE;
    }
    if ((argv = mprAlloc(((rec->nfields * 2) + 2) * sizeof(cchar*))) == 0) {
        return MPR_ERR_MEMORY;
    }
    argc = 0;

    buf = mprCreateBuf(0, 0);
    if (rec->id) {
        mprPutToBuf(buf, "UPDATE %s SET ", rec->tableName);
        for (f = 0; f < rec->nfields; f++) {
            fp = &rec->fields[f];
            mprPutToBuf(buf, "%s = ?, ", fp->name);
            argv[argc++] = mapSdbValue(fp->value, fp->type);
        }
        mprAdjustBufEnd(buf, -2);
        mprPutStringToBuf(buf, " WHERE id = ?;");
        argv[argc++] = rec->id;

    } else {
        mprPutToBuf(buf, "INSERT INTO %s (", rec->tableName);
        for (f = 1; f < rec->nfields; f++) {
            fp = &rec->fields[f];
            mprPutToBuf(buf, "%s,", fp->name);
        }
        mprAdjustBufEnd(buf, -1);
        mprPutStringToBuf(buf, ") VALUES (");
        for (f = 1; f < rec->nfields; f++) {
            mprPutStringToBuf(buf, "?,");
            fp = &rec->fields[f];
            argv[argc++] = mapSdbValue(fp->value, fp->type);
        }
        mprAdjustBufEnd(buf, -1);
        mprPutCharToBuf(buf, ')');
    }
    argv[argc] = NULL;

    if (queryArgv(edi, mprGetBufStart(buf), argc, argv, NULL) == 0) {
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


PUBLIC cchar *sdbGetLastError(Edi *edi)
{
    return ((Sdb*) edi)->edi.errMsg;
}


/*********************************** Support *******************************/

static EdiGrid *query(Edi *edi, cchar *cmd, ...)
{
    EdiGrid     *grid;
    va_list     args;

    va_start(args, cmd);
    grid = queryv(edi, cmd, 0, NULL, args);
    va_end(args);
    return grid;
}


/*
    Vars are ignored in queryv if argc != 0. Defined here just to satisify old compilers.
 */
static EdiGrid *queryArgv(Edi *edi, cchar *cmd, int argc, cchar **argv, ...)
{
    va_list     vargs;
    EdiGrid     *grid;

    va_start(vargs, argv);
    grid = queryv(edi, cmd, argc, argv, vargs);
    va_end(vargs);
    return grid;
}


/*
    This function supports parameterized queries. Provide parameters for query via either argc+argv or vargs.
    Strongly recommended to use parameterized queries to lessen SQL injection risk.
 */
static EdiGrid *queryv(Edi *edi, cchar *cmd, int argc, cchar **argv, va_list vargs)
{
    Sdb             *sdb;
    sqlite3         *db;
    sqlite3_stmt    *stmt;
    EdiGrid         *grid;
    EdiRec          *rec, *schema;
    MprList         *result;
    char            *tableName;
    cchar           *tail, *colName, *value, *defaultTableName, *arg;
    ssize           len;
    int             r, nrows, i, ncol, rc, retries, index, type;

    assert(edi);
    assert(cmd && *cmd);

    sdb = (Sdb*) edi;
    retries = 0;
    sdb->edi.errMsg = 0;

    if ((db = sdb->db) == 0) {
        sdbError(edi, "Database '%s' is closed", sdb->edi.path);
        return 0;
    }
    if ((result = mprCreateList(0, MPR_LIST_STABLE)) == 0) {
        return 0;
    }
    defaultTableName = 0;
    rc = SQLITE_OK;
    nrows = 0;

    while (cmd && *cmd && (rc == SQLITE_OK || (rc == SQLITE_SCHEMA && ++retries < 2))) {
        stmt = 0;
        mprLog("info esp sdb", 4, "SQL: %s", cmd);
        rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, &tail);
        if (rc != SQLITE_OK) {
            sdbDebug(edi, 2, "SDB: cannot prepare command: %s, error: %s", cmd, sqlite3_errmsg(db));
            continue;
        }
        if (stmt == 0) {
            /* Comment or white space */
            cmd = tail;
            continue;
        }
        if (argc == 0) {
            for (index = 0; (arg = va_arg(vargs, cchar*)) != 0; index++) {
                if (sqlite3_bind_text(stmt, index + 1, arg, -1, 0) != SQLITE_OK) {
                    sdbError(edi, "SDB: cannot bind to arg: %d, %s, error: %s", index + 1, arg, sqlite3_errmsg(db));
                    return 0;
                }
            }
        } else if (argv) {
            for (index = 0; index < argc; index++) {
                if (sqlite3_bind_text(stmt, index + 1, argv[index], -1, 0) != SQLITE_OK) {
                    sdbError(edi, "SDB: cannot bind to arg: %d, %s, error: %s", index + 1, argv[index], sqlite3_errmsg(db));
                    return 0;
                }
            }
        }
        ncol = sqlite3_column_count(stmt);
        for (nrows = 0; ; nrows++) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                tableName = (char*) sqlite3_column_table_name(stmt, 0);
                if ((rec = createBareRec(edi, tableName, ncol)) == 0) {
                    sqlite3_finalize(stmt);
                    return 0;
                }
                if (defaultTableName == 0) {
                    defaultTableName = rec->tableName;
                }
                mprAddItem(result, rec);
                for (i = 0; i < ncol; i++) {
                    colName = sqlite3_column_name(stmt, i);
                    value = (cchar*) sqlite3_column_text(stmt, i);
                    if (tableName && strcmp(tableName, defaultTableName) != 0) {
                        len = strlen(tableName) + 1;
                        tableName = sjoin("_", tableName, colName, NULL);
                        tableName[len] = toupper((uchar) tableName[len]);
                    }
                    if (tableName && ((schema = getSchema(edi, tableName)) != 0)) {
                        rec->fields[i] = makeRecField(value, colName, schema->fields[i].type);
                    } else {
                        type = sqlite3_column_type(stmt, i);
                        rec->fields[i] = makeRecField(value, colName, mapSqliteTypeToEdiType(type));
                    }
                    if (smatch(colName, "id")) {
                        rec->fields[i].flags |= EDI_KEY;
                        rec->id = rec->fields[i].value;
                    }
                }
            } else {
                rc = sqlite3_finalize(stmt);
                stmt = 0;
                if (rc != SQLITE_SCHEMA) {
                    retries = 0;
                    for (cmd = tail; isspace((uchar) *cmd); cmd++) {}
                }
                break;
            }
        }
    }
    if (stmt) {
        rc = sqlite3_finalize(stmt);
    }
    if (rc != SQLITE_OK) {
        if (rc == sqlite3_errcode(db)) {
            sdbDebug(edi, 2, "SDB: cannot run query: %s, error: %s", cmd, sqlite3_errmsg(db));
        } else {
            sdbDebug(edi, 2, "SDB: unspecified SQL error for: %s", cmd);
        }
        return 0;
    }
    if ((grid = ediCreateBareGrid(edi, defaultTableName, nrows)) == 0) {
        return 0;
    }
    for (r = 0; r < nrows; r++) {
        grid->records[r] = mprGetItem(result, r);
    }
    return grid;
}


static EdiRec *createBareRec(Edi *edi, cchar *tableName, int nfields)
{
    EdiRec  *rec;

    if ((rec = mprAllocBlock(sizeof(EdiRec) + sizeof(EdiField) * nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(rec, (MprManager) ediManageEdiRec);
    rec->edi = edi;
    rec->tableName = sclone(tableName);
    rec->nfields = nfields;
    return rec;
}


static EdiField makeRecField(cchar *value, cchar *name, int type)
{
    EdiField    f;

    f.valid = 1;
    f.value = sclone(value);
    f.name = sclone(name);
    f.type = type;
    f.flags = 0;
    return f;
}


static void *allocBlock(int size)
{
    return palloc(size);
}


static void freeBlock(void *ptr)
{
    pfree(ptr);
}


static void *reallocBlock(void *ptr, int size)
{
    return prealloc(ptr, size);
}


static int blockSize(void *ptr)
{
    return (int) mprGetBlockSize(ptr);
}


static int roundBlockSize(int size)
{
    return MPR_ALLOC_ALIGN(size);
}


static int initAllocator(void *data)
{
    return 0;
}


static void termAllocator(void *data)
{
}


struct sqlite3_mem_methods memMethods = {
    allocBlock, freeBlock, reallocBlock, blockSize, roundBlockSize, initAllocator, termAllocator, NULL
};


static cchar *mapToSqlType(int type)
{
    assert(0 < type && type < EDI_TYPE_MAX);
    return dataTypeToSqlType[type];
}


static int mapToEdiType(cchar *type)
{
    int     i;

    for (i = 0; i < EDI_TYPE_MAX; i++) {
        if (smatch(dataTypeToSqlType[i], type)) {
            return i;
        }
    }
    mprLog("error esp sdb", 0, "Cannot find type %s", type);
    return 0;
}


static int mapSqliteTypeToEdiType(int type)
{
    if (type == SQLITE_INTEGER) {
        return EDI_TYPE_INT;
    } else if (type == SQLITE_FLOAT) {
        return EDI_TYPE_FLOAT;
    } else if (type == SQLITE_TEXT) {
        return EDI_TYPE_TEXT;
    } else if (type == SQLITE_BLOB) {
        return EDI_TYPE_BINARY;
    } else if (type == SQLITE_NULL) {
        return EDI_TYPE_TEXT;
    }
    mprLog("error esp sdb", 0, "Cannot find query type %d", type);
    return 0;
}


static bool validName(cchar *str)
{
    cchar   *cp;

    if (!str) {
        return 0;
    }
    if (!isalpha(*str) && *str != '_') {
        return 0;
    }
    for (cp = &str[1]; *cp && (isalnum((uchar) *cp) || *cp == '_' || *cp == '$'); cp++) {}
    return (*cp == '\0');
}


static void sdbError(Edi *edi, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    edi->errMsg = sfmtv(fmt, args);
    va_end(args);
    mprLog("error esp sdb", 0, "%s", edi->errMsg);
}


static void sdbDebug(Edi *edi, int level, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    edi->errMsg = sfmtv(fmt, args);
    va_end(args);
    mprDebug("debug esp sdb", level, "%s", edi->errMsg);
}


/*********************************** Factory *******************************/

static void initSqlite(void)
{
    mprGlobalLock();
    if (!sqliteInitialized) {
        sqlite3_config(SQLITE_CONFIG_MALLOC, &memMethods);
        sqlite3_config(SQLITE_CONFIG_SERIALIZED);
        if (sqlite3_initialize() != SQLITE_OK) {
            mprLog("error esp sdb", 0, "Cannot initialize SQLite");
            return;
        }
        sqliteInitialized = 1;
    }
    mprGlobalUnlock();
}

#else
/* To prevent ar/ranlib warnings */
PUBLIC void sdbDummy(void) {}
#endif /* ME_COM_SQLITE */

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under a commercial license. Consult the LICENSE.md
    distributed with this software for full details and copyrights.
 */

#endif /* ME_COM_ESP */
