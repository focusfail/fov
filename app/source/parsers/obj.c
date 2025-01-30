#include "parsers/obj.h"

#include "log.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_obj(model_t* m, const char* fp)
{
    FILE* file = fopen(fp, "r");
    if (!file) {
        log_fatal("Failed to open .obj file to parse %s", fp);
    }

    int  face_format = -1;
    char line[96];
    long line_num = 1;

    while (fgets(line, sizeof(line), file)) {
        // Check buffer limits
        if (m->vertex_count >= MAX_VERTICES * 3 || m->indice_count >= MAX_INDICES) {
            log_error("Buffer limits exceeded at line %ld", line_num);
            break;
        }

        switch (line[0]) {
        case 'v': {
            if (line[1] == ' ') {
                if (sscanf(line + 2, "%lf %lf %lf", &m->vertices[m->vertex_count], &m->vertices[m->vertex_count + 1],
                           &m->vertices[m->vertex_count + 2])
                    != 3)
                {
                    log_warn("Invalid vertex at line %ld", line_num);
                    continue;
                }
                m->vertex_count += 3;
            } else if (line[1] == 'n') {
                // if (sscanf(line + 3, "%f %f %f", &m->normals[m->normal_count], &m->normals[m->normal_count + 1],
                //            &m->normals[m->normal_count + 2])
                //     != 3)
                // {
                //     log_warn("Invalid normal at line %ld", line_num);
                //     continue;
                // }
                // m->normal_count += 3;
            } else if (line[1] == 't') {
                // if (sscanf(line + 3, "%f %f", &m->texcrds[m->texcrd_count], &m->texcrds[m->texcrd_count + 1]) != 2) {
                //     log_warn("Invalid texcoord at line %ld", line_num);
                //     continue;
                // }
                // m->texcrd_count += 2;
            }
            break;
        }
        case 'f': {
            if (face_format < 0) {
                face_format = strchr(line, '/') ? 1 : 2;
            }

            unsigned int v1, v2, v3;
            unsigned int vt1, vt2, vt3;
            unsigned int vn1, vn2, vn3;
            int          result;

            if (face_format == 1) {
                result
                    = sscanf(line + 2, "%u/%u/%u %u/%u/%u %u/%u/%u", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);
                if (result != 9) {
                    log_warn("Invalid face format at line %ld", line_num);
                    continue;
                }
            } else {
                result = sscanf(line + 2, "%u %u %u", &v1, &v2, &v3);
                if (result != 3) {
                    log_warn("Invalid face format at line %ld", line_num);
                    continue;
                }
            }

            // Validate indices
            if (v1 == 0 || v2 == 0 || v3 == 0 || v1 > (unsigned int)m->vertex_count / 3
                || v2 > (unsigned int)m->vertex_count / 3 || v3 > (unsigned int)m->vertex_count / 3)
            {
                log_warn("Invalid vertex index at line %ld", line_num);
                continue;
            }

            m->indices[m->indice_count++] = v1 - 1;
            m->indices[m->indice_count++] = v2 - 1;
            m->indices[m->indice_count++] = v3 - 1;
            break;
        }
        }
        line_num++;
    }

    float sz = model_get_size_mb(m);
    log_info("Loaded model [verts: %d;  approx. size: %.4fMB]", m->vertex_count, sz);

    fclose(file);
}