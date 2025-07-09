#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libiec61850/iec61850_common.h>
#include <libiec61850/mms_client_connection.h>

static void print_json_string(const char* str) {
    putchar('"');
    while (*str) {
        if (*str == '"' || *str == '\\') putchar('\\');
        putchar(*str++);
    }
    putchar('"');
}

static const char* mms_error_to_string(MmsError e)
{
    switch (e) {
        case MMS_ERROR_NONE: return "MMS_ERROR_NONE";
        case MMS_ERROR_CONNECTION_REJECTED: return "MMS_ERROR_CONNECTION_REJECTED";
        case MMS_ERROR_CONNECTION_LOST: return "MMS_ERROR_CONNECTION_LOST";
        case MMS_ERROR_SERVICE_TIMEOUT: return "MMS_ERROR_SERVICE_TIMEOUT";
        case MMS_ERROR_PARSING_RESPONSE: return "MMS_ERROR_PARSING_RESPONSE";
        case MMS_ERROR_HARDWARE_FAULT: return "MMS_ERROR_HARDWARE_FAULT";
        case MMS_ERROR_CONCLUDE_REJECTED: return "MMS_ERROR_CONCLUDE_REJECTED";
        case MMS_ERROR_INVALID_ARGUMENTS: return "MMS_ERROR_INVALID_ARGUMENTS";
        case MMS_ERROR_OUTSTANDING_CALL_LIMIT: return "MMS_ERROR_OUTSTANDING_CALL_LIMIT";
        case MMS_ERROR_OTHER: return "MMS_ERROR_OTHER";
        case MMS_ERROR_VMDSTATE_OTHER: return "MMS_ERROR_VMDSTATE_OTHER";
        case MMS_ERROR_APPLICATION_REFERENCE_OTHER: return "MMS_ERROR_APPLICATION_REFERENCE_OTHER";
        case MMS_ERROR_DEFINITION_OTHER: return "MMS_ERROR_DEFINITION_OTHER";
        case MMS_ERROR_DEFINITION_INVALID_ADDRESS: return "MMS_ERROR_DEFINITION_INVALID_ADDRESS";
        case MMS_ERROR_DEFINITION_TYPE_UNSUPPORTED: return "MMS_ERROR_DEFINITION_TYPE_UNSUPPORTED";
        case MMS_ERROR_DEFINITION_TYPE_INCONSISTENT: return "MMS_ERROR_DEFINITION_TYPE_INCONSISTENT";
        case MMS_ERROR_DEFINITION_OBJECT_UNDEFINED: return "MMS_ERROR_DEFINITION_OBJECT_UNDEFINED";
        case MMS_ERROR_DEFINITION_OBJECT_EXISTS: return "MMS_ERROR_DEFINITION_OBJECT_EXISTS";
        case MMS_ERROR_DEFINITION_OBJECT_ATTRIBUTE_INCONSISTENT: return "MMS_ERROR_DEFINITION_OBJECT_ATTRIBUTE_INCONSISTENT";
        case MMS_ERROR_RESOURCE_OTHER: return "MMS_ERROR_RESOURCE_OTHER";
        case MMS_ERROR_RESOURCE_CAPABILITY_UNAVAILABLE: return "MMS_ERROR_RESOURCE_CAPABILITY_UNAVAILABLE";
        case MMS_ERROR_SERVICE_OTHER: return "MMS_ERROR_SERVICE_OTHER";
        case MMS_ERROR_SERVICE_OBJECT_CONSTRAINT_CONFLICT: return "MMS_ERROR_SERVICE_OBJECT_CONSTRAINT_CONFLICT";
        case MMS_ERROR_SERVICE_PREEMPT_OTHER: return "MMS_ERROR_SERVICE_PREEMPT_OTHER";
        case MMS_ERROR_TIME_RESOLUTION_OTHER: return "MMS_ERROR_TIME_RESOLUTION_OTHER";
        case MMS_ERROR_ACCESS_OTHER: return "MMS_ERROR_ACCESS_OTHER";
        case MMS_ERROR_ACCESS_OBJECT_NON_EXISTENT: return "MMS_ERROR_ACCESS_OBJECT_NON_EXISTENT";
        case MMS_ERROR_ACCESS_OBJECT_ACCESS_UNSUPPORTED: return "MMS_ERROR_ACCESS_OBJECT_ACCESS_UNSUPPORTED";
        case MMS_ERROR_ACCESS_OBJECT_ACCESS_DENIED: return "MMS_ERROR_ACCESS_OBJECT_ACCESS_DENIED";
        case MMS_ERROR_ACCESS_OBJECT_INVALIDATED: return "MMS_ERROR_ACCESS_OBJECT_INVALIDATED";
        case MMS_ERROR_ACCESS_OBJECT_VALUE_INVALID: return "MMS_ERROR_ACCESS_OBJECT_VALUE_INVALID";
        case MMS_ERROR_ACCESS_TEMPORARILY_UNAVAILABLE: return "MMS_ERROR_ACCESS_TEMPORARILY_UNAVAILABLE";
        case MMS_ERROR_FILE_OTHER: return "MMS_ERROR_FILE_OTHER";
        case MMS_ERROR_FILE_FILENAME_AMBIGUOUS: return "MMS_ERROR_FILE_FILENAME_AMBIGUOUS";
        case MMS_ERROR_FILE_FILE_BUSY: return "MMS_ERROR_FILE_FILE_BUSY";
        case MMS_ERROR_FILE_FILENAME_SYNTAX_ERROR: return "MMS_ERROR_FILE_FILENAME_SYNTAX_ERROR";
        case MMS_ERROR_FILE_CONTENT_TYPE_INVALID: return "MMS_ERROR_FILE_CONTENT_TYPE_INVALID";
        case MMS_ERROR_FILE_POSITION_INVALID: return "MMS_ERROR_FILE_POSITION_INVALID";
        case MMS_ERROR_FILE_FILE_ACCESS_DENIED: return "MMS_ERROR_FILE_FILE_ACCESS_DENIED";
        case MMS_ERROR_FILE_FILE_NON_EXISTENT: return "MMS_ERROR_FILE_FILE_NON_EXISTENT";
        case MMS_ERROR_FILE_DUPLICATE_FILENAME: return "MMS_ERROR_FILE_DUPLICATE_FILENAME";
        case MMS_ERROR_FILE_INSUFFICIENT_SPACE_IN_FILESTORE: return "MMS_ERROR_FILE_INSUFFICIENT_SPACE_IN_FILESTORE";
        case MMS_ERROR_REJECT_OTHER: return "MMS_ERROR_REJECT_OTHER";
        case MMS_ERROR_REJECT_UNKNOWN_PDU_TYPE: return "MMS_ERROR_REJECT_UNKNOWN_PDU_TYPE";
        case MMS_ERROR_REJECT_INVALID_PDU: return "MMS_ERROR_REJECT_INVALID_PDU";
        case MMS_ERROR_REJECT_UNRECOGNIZED_SERVICE: return "MMS_ERROR_REJECT_UNRECOGNIZED_SERVICE";
        case MMS_ERROR_REJECT_UNRECOGNIZED_MODIFIER: return "MMS_ERROR_REJECT_UNRECOGNIZED_MODIFIER";
        case MMS_ERROR_REJECT_REQUEST_INVALID_ARGUMENT: return "MMS_ERROR_REJECT_REQUEST_INVALID_ARGUMENT";
        default: return "MMS_ERROR_UNKNOWN";
    }
}

static long decode_bitstring_as_int(MmsValue* value) {
    if (!value || MmsValue_getType(value) != MMS_BIT_STRING) return -1;
    int size = MmsValue_getBitStringByteSize(value);
    uint8_t* buf = (uint8_t*)MmsValue_getOctetStringBuffer(value);
    if (size == 0) return 0;
    if (size == 1) return buf[0];
    if (size == 2) return ((long)buf[0] << 8) | buf[1];
    return buf[0];
}

static const char* tase2_flat_type(MmsValue* value) {
    if (!value) return "unknown";
    MmsType t = MmsValue_getType(value);
    if (t == MMS_FLOAT) return "Real";
    if (t == MMS_INTEGER) return "Discrete_Or_State";
    if (t == MMS_UNSIGNED) return "Unsigned";
    if (t == MMS_BOOLEAN) return "Bool";
    if (t == MMS_STRING || t == MMS_VISIBLE_STRING) return "String";
    if (t == MMS_BIT_STRING) return "Discrete_Or_State";
    if (t == MMS_STRUCTURE) {
        int n = MmsValue_getArraySize(value);
        if (n > 0) {
            MmsType t0 = MmsValue_getType(MmsValue_getElement(value, 0));
            if (t0 == MMS_FLOAT) return "Real";
            if (t0 == MMS_INTEGER) return "Discrete_Or_State";
            if (t0 == MMS_UNSIGNED) return "Unsigned";
            if (t0 == MMS_BIT_STRING && decode_bitstring_as_int(MmsValue_getElement(value, 0)) >= 0)
                return "Discrete_Or_State";
            if (n >= 2) {
                MmsType t1 = MmsValue_getType(MmsValue_getElement(value, 1));
                if (t0 == MMS_INTEGER && t1 == MMS_BIT_STRING)
                    return "Discrete_Or_State";
            }
        }
    }
    return "unknown";
}

static const char* mms_type_to_string(MmsType t)
{
    switch (t) {
        case MMS_ARRAY:           return "MMS_ARRAY";
        case MMS_STRUCTURE:       return "MMS_STRUCTURE";
        case MMS_BOOLEAN:         return "MMS_BOOLEAN";
        case MMS_BIT_STRING:      return "MMS_BIT_STRING";
        case MMS_INTEGER:         return "MMS_INTEGER";
        case MMS_UNSIGNED:        return "MMS_UNSIGNED";
        case MMS_FLOAT:           return "MMS_FLOAT";
        case MMS_OCTET_STRING:    return "MMS_OCTET_STRING";
        case MMS_VISIBLE_STRING:  return "MMS_VISIBLE_STRING";
        case MMS_GENERALIZED_TIME:return "MMS_GENERALIZED_TIME";
        case MMS_BINARY_TIME:     return "MMS_BINARY_TIME";
        case MMS_BCD:             return "MMS_BCD";
        case MMS_OBJ_ID:          return "MMS_OBJ_ID";
        case MMS_STRING:          return "MMS_STRING";
        case MMS_UTC_TIME:        return "MMS_UTC_TIME";
        default:                  return "UNKNOWN";
    }
}

static const char* get_structure_main_type_string(MmsValue* value)
{
    if (!value) return "null";
    MmsType t = MmsValue_getType(value);

    if (t == MMS_STRUCTURE) {
        int n = MmsValue_getArraySize(value);
        if (n > 0) {
            MmsType t0 = MmsValue_getType(MmsValue_getElement(value, 0));
            return mms_type_to_string(t0);
        } else {
            return "UNKNOWN";
        }
    } else {
        return mms_type_to_string(t);
    }
}

static void print_main_value(MmsValue* value) {
    if (!value) {
        printf("null");
        return;
    }

    MmsType t = MmsValue_getType(value);

    if (t == MMS_FLOAT) {
        printf("%f", MmsValue_toDouble(value));
    } else if (t == MMS_INTEGER) {
        printf("%lld", (long long)MmsValue_toInt64(value));
    } else if (t == MMS_UNSIGNED) {
        printf("%llu", (unsigned long long)MmsValue_toInt64(value));
    } else if (t == MMS_BOOLEAN) {
        printf(MmsValue_getBoolean(value) ? "true" : "false");
    } else if (t == MMS_STRING || t == MMS_VISIBLE_STRING) {
        print_json_string(MmsValue_toString(value));
    } else if (t == MMS_BIT_STRING) {
        long v = decode_bitstring_as_int(value);
        if (v >= 0)
            printf("%ld", v);
        else {
            int sz = MmsValue_getBitStringByteSize(value);
            uint8_t* buf = (uint8_t*)MmsValue_getOctetStringBuffer(value);
            printf("\"0x");
            for (int i = 0; i < sz; ++i)
                printf("%02x", buf[i]);
            printf("\"");
        }
    } else if (t == MMS_STRUCTURE) {
        int n = MmsValue_getArraySize(value);
        if (n > 0) {
            MmsValue* elem0 = MmsValue_getElement(value, 0);
            MmsType t0 = MmsValue_getType(elem0);
            if (t0 == MMS_FLOAT) {
                printf("%f", MmsValue_toDouble(elem0));
                return;
            } else if (t0 == MMS_INTEGER) {
                printf("%lld", (long long)MmsValue_toInt64(elem0));
                return;
            } else if (t0 == MMS_UNSIGNED) {
                printf("%llu", (unsigned long long)MmsValue_toInt64(elem0));
                return;
            } else if (t0 == MMS_BIT_STRING) {
                long v = decode_bitstring_as_int(elem0);
                if (v >= 0) {
                    printf("%ld", v);
                    return;
                }
            }
            if (n >= 2) {
                MmsType t1 = MmsValue_getType(MmsValue_getElement(value, 1));
                if (t0 == MMS_INTEGER && t1 == MMS_BIT_STRING) {
                    printf("%lld", (long long)MmsValue_toInt64(elem0));
                    return;
                }
                if (t0 == MMS_BIT_STRING && t1 == MMS_BIT_STRING) {
                    long v = decode_bitstring_as_int(elem0);
                    if (v >= 0) {
                        printf("%ld", v);
                        return;
                    }
                }
            }
        }
        if (n > 1) {
            MmsValue* elem1 = MmsValue_getElement(value, 1);
            if (MmsValue_getType(elem1) == MMS_INTEGER) {
                printf("%lld", (long long)MmsValue_toInt64(elem1));
                return;
            }
        }
        printf("[");
        for (int i=0; i < n; ++i) {
            if (i > 0) printf(", ");
            print_main_value(MmsValue_getElement(value, i));
        }
        printf("]");
    } else if (t == MMS_ARRAY) {
        int n = MmsValue_getArraySize(value);
        printf("[");
        for (int i=0; i < n; ++i) {
            if (i > 0) printf(", ");
            print_main_value(MmsValue_getElement(value,i));
        }
        printf("]");
    } else if (t == MMS_OCTET_STRING) {
        int sz = MmsValue_getOctetStringSize(value);
        uint8_t* buf = (uint8_t*)MmsValue_getOctetStringBuffer(value);
        printf("\"0x");
        for (int i = 0; i < sz; ++i)
            printf("%02X", buf[i]);
        printf("\"");
    } else {
        char buf[128];
        MmsValue_printToBuffer(value, buf, sizeof(buf));
        print_json_string(buf);
    }
}

static void print_json_flat_value(MmsValue* value, const char* name, MmsError error_code)
{
    printf("        {\n          \"name\": ");
    print_json_string(name);

    if (error_code == MMS_ERROR_NONE) {
        printf(",\n          \"type\": \"%s\"", value ? tase2_flat_type(value) : "null");
        printf(",\n          \"mms_type\": \"%s\"", value ? get_structure_main_type_string(value) : "null");
        printf(",\n          \"value\": ");
        if (value)
            print_main_value(value);
        else
            printf("null");
    } else {
        printf(",\n          \"type\": \"unknown\"");
        printf(",\n          \"mms_type\": \"unknown\"");
        printf(",\n          \"error\": ");
        print_json_string(mms_error_to_string(error_code));
    }
    printf("\n        }");
}

static const char* ignore_vars[] = { "Bilateral_Table_ID", /* more names, or remove */ NULL };

static int is_ignored(const char* name) {
    for (int i = 0; ignore_vars[i] != NULL; ++i) {
        if (strcasecmp(name, ignore_vars[i]) == 0)
            return 1;
    }
    return 0;
}

static void print_value_as_string(MmsValue* value)
{
    if (!value) {
        printf("null");
        return;
    }

    char buf[128];

    MmsType t = MmsValue_getType(value);
    switch (t) {
        case MMS_INTEGER:
            snprintf(buf, sizeof(buf), "%lld", (long long) MmsValue_toInt64(value));
            print_json_string(buf);
            break;
        case MMS_UNSIGNED:
            snprintf(buf, sizeof(buf), "%llu", (unsigned long long) MmsValue_toInt64(value));
            print_json_string(buf);
            break;
        case MMS_FLOAT:
            snprintf(buf, sizeof(buf), "%g", MmsValue_toDouble(value));
            print_json_string(buf);
            break;
        case MMS_BOOLEAN:
            print_json_string(MmsValue_getBoolean(value) ? "true" : "false");
            break;
        case MMS_STRING:
        case MMS_VISIBLE_STRING:
            print_json_string(MmsValue_toString(value));
            break;
        case MMS_BIT_STRING: {
            long v = decode_bitstring_as_int(value);
            if (v >= 0) {
                snprintf(buf, sizeof(buf), "%ld", v);
                print_json_string(buf);
            } else {
                int sz = MmsValue_getBitStringByteSize(value);
                uint8_t* bb = (uint8_t*)MmsValue_getOctetStringBuffer(value);
                int n = snprintf(buf, sizeof(buf), "0x");
                for (int i = 0; i < sz && n < (int)sizeof(buf)-2; ++i)
                    n += snprintf(buf+n, sizeof(buf)-n, "%02x", bb[i]);
                print_json_string(buf);
            }
            break;
        }
        case MMS_STRUCTURE: {
            int n = MmsValue_getArraySize(value);
            if (n > 0) {
                print_value_as_string(MmsValue_getElement(value, 0));
            } else {
                printf("null");
            }
            break;
        }
        default:
            MmsValue_printToBuffer(value, buf, sizeof(buf));
            print_json_string(buf);
    }
}

static void print_tase2_version_as_string(MmsValue* value)
{
    if (!value) {
        printf("null");
        return;
    }

    if (MmsValue_getType(value) == MMS_STRUCTURE && MmsValue_getArraySize(value) == 2) {
        MmsValue* major = MmsValue_getElement(value, 0);
        MmsValue* minor = MmsValue_getElement(value, 1);
        if ((MmsValue_getType(major) == MMS_INTEGER || MmsValue_getType(major) == MMS_UNSIGNED) &&
            (MmsValue_getType(minor) == MMS_INTEGER || MmsValue_getType(minor) == MMS_UNSIGNED)) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%lld.%lld",
                     (long long) MmsValue_toInt64(major),
                     (long long) MmsValue_toInt64(minor));
            print_json_string(buf);
            return;
        }
    }
    print_value_as_string(value);
}

static void print_server_identity_json(MmsConnection con)
{
    MmsError error;
    MmsServerIdentity* id = MmsConnection_identify(con, &error);
    printf("  \"server_identity\": ");
    if (id != NULL && error == MMS_ERROR_NONE) {
        printf("{\n    \"vendor\": ");
        print_json_string(id->vendorName ? id->vendorName : "");
        printf(",\n    \"model\": ");
        print_json_string(id->modelName ? id->modelName : "");
        printf(",\n    \"revision\": ");
        print_json_string(id->revision ? id->revision : "");
        printf("\n  },\n");
    } else {
        printf("null,\n");
    }
}

static void print_help(const char* prog_name) {
    printf("Usage: %s [hostname [port]]\n", prog_name);
    printf("Query an MMS server and print information as JSON.\n\n");
    printf("Options:\n");
    printf("  --help        Print this help message and exit.\n");
    printf("\n");
    printf("Arguments:\n");
    printf("  hostname      IP address or hostname of the server (default: localhost)\n");
    printf("  tcp port      TCP port to connect (default: 102)\n");
}

int main(int argc, char** argv) {
    char* hostname = NULL;
    int tcpPort = 102;
    MmsError error;
    int returnCode = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }
    }

    if (argc > 1) {
        hostname = argv[1];
        if (argc > 2) {
            tcpPort = atoi(argv[2]);
            if (tcpPort <= 0 || tcpPort > 65535) {
                fprintf(stderr, "invalid tcp port: %s\n", argv[2]);
                return EXIT_FAILURE;
            }
        }
    } else {
        hostname = "localhost";
    }

    MmsConnection con = MmsConnection_create();
    if (!MmsConnection_connect(con, &error, hostname, tcpPort)) {
        printf("{\n  \"server_identity\": null\n}\n");
        MmsConnection_destroy(con);
        return EXIT_FAILURE;
    }

    printf("{\n");

    print_server_identity_json(con);

    MmsValue* tase2v = MmsConnection_readVariable(con, &error, NULL, "TASE2_Version");
    MmsValue* tase2sf = MmsConnection_readVariable(con, &error, NULL, "Supported_Features");

    printf("  \"TASE2_Version\": ");
    if (tase2v) print_tase2_version_as_string(tase2v); else printf("null");
    if (tase2v) MmsValue_delete(tase2v);
    printf(",\n");

    printf("  \"Supported_Features\": ");
    if (tase2sf) print_main_value(tase2sf); else printf("null");
    if (tase2sf) MmsValue_delete(tase2sf);
    printf(",\n");

    printf("  \"domains\": [\n");

    LinkedList domains = MmsConnection_getDomainNames(con, &error);
    if (error != MMS_ERROR_NONE || domains == NULL) {
        fprintf(stderr, "\"error\": \"domain-list failed\"}\n");
        goto cleanup;
    }

    LinkedList domainElem = LinkedList_getNext(domains);
    int firstDomain = 1;

    while (domainElem != NULL) {
        if (!firstDomain) printf(",\n");
        firstDomain = 0;

        char* domainName = (char*)domainElem->data;
        printf("    {\n      \"name\": ");
        print_json_string(domainName);

        MmsValue* bilateral_table_id_val = MmsConnection_readVariable(con, &error, domainName, "Bilateral_Table_ID");
        printf(",\n      \"bilateral_table_id\": ");
        if (bilateral_table_id_val) {
            print_main_value(bilateral_table_id_val);
            MmsValue_delete(bilateral_table_id_val);
        } else {
            printf("null");
        }

        printf(",\n      \"variables\": [\n");

        LinkedList variables = MmsConnection_getDomainVariableNames(con, &error, domainName);
        LinkedList varElem = LinkedList_getNext(variables);
        int firstVar = 1;
        while (varElem != NULL) {
            if (!firstVar) printf(",\n");
            char* varName = (char*)varElem->data;

            if (is_ignored(varName)) {
                varElem = LinkedList_getNext(varElem);
                continue;
            }

            MmsError localErr = MMS_ERROR_NONE;
            MmsValue* value = MmsConnection_readVariable(con, &localErr, domainName, varName);

            print_json_flat_value(value, varName, localErr);

            if (value) MmsValue_delete(value);
            varElem = LinkedList_getNext(varElem);
            firstVar = 0;
        }
        printf("\n      ]\n    }");
        LinkedList_destroy(variables);
        domainElem = LinkedList_getNext(domainElem);
    }
    printf("\n  ]\n}\n");
    LinkedList_destroy(domains);

cleanup:
    MmsConnection_destroy(con);
    return returnCode;
}

