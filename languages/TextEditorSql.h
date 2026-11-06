//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	GetLanguageDefinitionSql
//

static const TextEditor::Language* GetLanguageDefinitionSql() {
	static bool initialized = false;
	static TextEditor::Language language;

	if (!initialized) {
		language.name = "SQL";
		language.caseSensitive = false;
		language.singleLineComment = "--";
		language.commentStart = "/*";
		language.commentEnd = "*/";
		language.hasSingleQuotedStrings = true;
		language.hasDoubleQuotedStrings = true;
		language.stringEscape = '\\';

		static const char* const keywords[] = {
			"abs", "absent", "acos", "all", "allocate", "alter", "and", "any", "any_value", "are", "array", "array_agg",
			"array_max_cardinality", "as", "asensitive", "asin", "asymmetric", "at", "atan", "atomic", "authorization",
			"avg", "begin", "begin_frame", "begin_partition", "between", "bigint", "binary", "blob", "boolean", "both",
			"btrim", "by", "call", "called", "cardinality", "cascaded", "case", "cast", "ceil", "ceiling", "char",
			"character", "character_length", "char_length", "check", "classifier", "clob", "close", "coalesce", "collate",
			"collect", "column", "commit", "condition", "connect", "constraint", "contains", "convert", "copy", "corr",
			"corresponding", "cos", "cosh", "count", "covar_pop", "covar_samp", "create", "cross", "cube", "cume_dist",
			"current", "current_catalog", "current_date", "current_default_transform_group", "current_path", "current_role",
			"current_row", "current_schema", "current_time", "current_timestamp", "current_transform_group_for_type",
			"current_user", "cursor", "cycle", "date", "day", "deallocate", "dec", "decfloat", "decimal", "declare",
			"default", "define", "delete", "dense_rank", "deref", "describe", "deterministic", "disconnect", "distinct",
			"double", "drop", "dynamic", "each", "element", "else", "empty", "end", "end-exec", "end_frame", "end_partition",
			"equals", "escape", "every", "except", "exec", "execute", "exists", "exp", "external", "extract", "false", "fetch",
			"filter", "first_value", "float", "floor", "for", "foreign", "frame_row", "free", "from", "full", "function",
			"fusion", "get", "global", "grant", "greatest", "group", "grouping", "groups", "having", "hold", "hour",
			"identity", "in", "indicator", "initial", "inner", "inout", "insensitive", "insert", "int", "integer",
			"intersect", "intersection", "interval", "into", "is", "join", "json", "json_array", "json_arrayagg",
			"json_exists", "json_object", "json_objectagg", "json_query", "json_scalar", "json_serialize", "json_table",
			"json_table_primitive", "json_value", "lag", "language", "large", "last_value", "lateral", "lead", "leading",
			"least", "left", "like", "like_regex", "limit", "listagg", "ln", "local", "localtime", "localtimestamp", "log", "log10",
			"lower", "lpad", "ltrim", "match", "matches", "match_number", "match_recognize", "max", "member", "merge", "method",
			"min", "minute", "mod", "modifies", "module", "month", "multiset", "national", "natural", "nchar", "nclob", "new",
			"no", "none", "normalize", "not", "nth_value", "ntile", "null", "nullif", "numeric", "occurrences_regex",
			"octet_length", "of", "offset", "old", "omit", "on", "one", "only", "open", "or", "order", "out", "outer", "over",
			"overlaps", "overlay", "parameter", "partition", "pattern", "per", "percent", "percentile_cont", "percentile_disc",
			"percent_rank", "period", "portion", "position", "position_regex", "power", "precedes", "precision", "prepare", "primary",
			"procedure", "ptf", "range", "rank", "reads", "real", "recursive", "ref", "references", "referencing", "regr_avgx",
			"regr_avgy", "regr_count", "regr_intercept", "regr_r2", "regr_slope", "regr_sxx", "regr_sxy", "regr_syy", "release",
			"result", "return", "returns", "revoke", "right", "rollback", "rollup", "row", "rows", "row_number", "rpad", "running",
			"savepoint", "scope", "scroll", "search", "second", "seek", "select", "sensitive", "session_user", "set", "show", "similar",
			"sin", "sinh", "skip", "smallint", "some", "specific", "specifictype", "sql", "sqlexception", "sqlstate", "sqlwarning", "sqrt",
			"start", "static", "stddev_pop", "stddev_samp", "submultiset", "subset", "substring", "substring_regex", "succeeds",
			"sum", "symmetric", "system", "system_time", "system_user", "table", "tablesample", "tan", "tanh", "then", "time",
			"timestamp", "timezone_hour", "timezone_minute", "to", "trailing", "translate", "translate_regex", "translation",
			"treat", "trigger", "trim", "trim_array", "true", "truncate", "uescape", "union", "unique", "unknown", "unnest",
			"update", "upper", "user", "using", "value", "values", "value_of", "varbinary", "varchar", "varying", "var_pop",
			"var_samp", "versioning", "when", "whenever", "where", "width_bucket", "window", "with", "within", "without", "year"
		};

		for (auto& keyword : keywords) { language.keywords.insert(keyword); }

		language.isPunctuation = isCStylePunctuation;
		language.getIdentifier = getCStyleIdentifier;
		language.getNumber = getCStyleNumber;
		initialized = true;
	}

	return &language;
}
