# **********************************************************************
# * Copyright (C) 2020 MX Authors
# *
# * Authors: Adrian
# *          MX Linux <http://mxlinux.org>
# *
# * This is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this package. If not, see <http://www.gnu.org/licenses/>.
# **********************************************************************/

QT       += widgets
CONFIG   += c++1z warn_on

TARGET = mx-dockmaker
TEMPLATE = app

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
    mainwindow.cpp \
    about.cpp \
    cmd.cpp \
    picklocation.cpp

HEADERS  += \
    mainwindow.h \
    version.h \
    about.h \
    cmd.h \
    picklocation.h

FORMS    += \
    mainwindow.ui \
    picklocation.ui

TRANSLATIONS += translations/mx-dockmaker_af.ts \
                translations/mx-dockmaker_am.ts \
                translations/mx-dockmaker_ar.ts \
                translations/mx-dockmaker_ast.ts \
                translations/mx-dockmaker_be.ts \
                translations/mx-dockmaker_bg.ts \
                translations/mx-dockmaker_bn.ts \
                translations/mx-dockmaker_bs_BA.ts \
                translations/mx-dockmaker_bs.ts \
                translations/mx-dockmaker_ca.ts \
                translations/mx-dockmaker_ceb.ts \
                translations/mx-dockmaker_co.ts \
                translations/mx-dockmaker_cs.ts \
                translations/mx-dockmaker_cy.ts \
                translations/mx-dockmaker_da.ts \
                translations/mx-dockmaker_de.ts \
                translations/mx-dockmaker_el.ts \
                translations/mx-dockmaker_en_GB.ts \
                translations/mx-dockmaker_en.ts \
                translations/mx-dockmaker_en_US.ts \
                translations/mx-dockmaker_eo.ts \
                translations/mx-dockmaker_es_ES.ts \
                translations/mx-dockmaker_es.ts \
                translations/mx-dockmaker_et.ts \
                translations/mx-dockmaker_eu.ts \
                translations/mx-dockmaker_fa.ts \
                translations/mx-dockmaker_fi_FI.ts \
                translations/mx-dockmaker_fil_PH.ts \
                translations/mx-dockmaker_fil.ts \
                translations/mx-dockmaker_fi.ts \
                translations/mx-dockmaker_fr_BE.ts \
                translations/mx-dockmaker_fr.ts \
                translations/mx-dockmaker_fy.ts \
                translations/mx-dockmaker_ga.ts \
                translations/mx-dockmaker_gd.ts \
                translations/mx-dockmaker_gl_ES.ts \
                translations/mx-dockmaker_gl.ts \
                translations/mx-dockmaker_gu.ts \
                translations/mx-dockmaker_ha.ts \
                translations/mx-dockmaker_haw.ts \
                translations/mx-dockmaker_he_IL.ts \
                translations/mx-dockmaker_he.ts \
                translations/mx-dockmaker_hi.ts \
                translations/mx-dockmaker_hr.ts \
                translations/mx-dockmaker_ht.ts \
                translations/mx-dockmaker_hu.ts \
                translations/mx-dockmaker_hy_AM.ts \
                translations/mx-dockmaker_hye.ts \
                translations/mx-dockmaker_hy.ts \
                translations/mx-dockmaker_id.ts \
                translations/mx-dockmaker_ie.ts \
                translations/mx-dockmaker_is.ts \
                translations/mx-dockmaker_it.ts \
                translations/mx-dockmaker_ja.ts \
                translations/mx-dockmaker_jv.ts \
                translations/mx-dockmaker_kab.ts \
                translations/mx-dockmaker_ka.ts \
                translations/mx-dockmaker_kk.ts \
                translations/mx-dockmaker_km.ts \
                translations/mx-dockmaker_kn.ts \
                translations/mx-dockmaker_ko.ts \
                translations/mx-dockmaker_ku.ts \
                translations/mx-dockmaker_ky.ts \
                translations/mx-dockmaker_lb.ts \
                translations/mx-dockmaker_lo.ts \
                translations/mx-dockmaker_lt.ts \
                translations/mx-dockmaker_lv.ts \
                translations/mx-dockmaker_mg.ts \
                translations/mx-dockmaker_mi.ts \
                translations/mx-dockmaker_mk.ts \
                translations/mx-dockmaker_ml.ts \
                translations/mx-dockmaker_mn.ts \
                translations/mx-dockmaker_mr.ts \
                translations/mx-dockmaker_ms.ts \
                translations/mx-dockmaker_mt.ts \
                translations/mx-dockmaker_my.ts \
                translations/mx-dockmaker_nb_NO.ts \
                translations/mx-dockmaker_nb.ts \
                translations/mx-dockmaker_ne.ts \
                translations/mx-dockmaker_nl_BE.ts \
                translations/mx-dockmaker_nl.ts \
                translations/mx-dockmaker_nn.ts \
                translations/mx-dockmaker_ny.ts \
                translations/mx-dockmaker_oc.ts \
                translations/mx-dockmaker_or.ts \
                translations/mx-dockmaker_pa.ts \
                translations/mx-dockmaker_pl.ts \
                translations/mx-dockmaker_ps.ts \
                translations/mx-dockmaker_pt_BR.ts \
                translations/mx-dockmaker_pt.ts \
                translations/mx-dockmaker_ro.ts \
                translations/mx-dockmaker_rue.ts \
                translations/mx-dockmaker_ru_RU.ts \
                translations/mx-dockmaker_ru.ts \
                translations/mx-dockmaker_rw.ts \
                translations/mx-dockmaker_sd.ts \
                translations/mx-dockmaker_si.ts \
                translations/mx-dockmaker_sk.ts \
                translations/mx-dockmaker_sl.ts \
                translations/mx-dockmaker_sm.ts \
                translations/mx-dockmaker_sn.ts \
                translations/mx-dockmaker_so.ts \
                translations/mx-dockmaker_sq.ts \
                translations/mx-dockmaker_sr.ts \
                translations/mx-dockmaker_st.ts \
                translations/mx-dockmaker_su.ts \
                translations/mx-dockmaker_sv.ts \
                translations/mx-dockmaker_sw.ts \
                translations/mx-dockmaker_ta.ts \
                translations/mx-dockmaker_te.ts \
                translations/mx-dockmaker_tg.ts \
                translations/mx-dockmaker_th.ts \
                translations/mx-dockmaker_tk.ts \
                translations/mx-dockmaker_tr.ts \
                translations/mx-dockmaker_tt.ts \
                translations/mx-dockmaker_ug.ts \
                translations/mx-dockmaker_uk.ts \
                translations/mx-dockmaker_ur.ts \
                translations/mx-dockmaker_uz.ts \
                translations/mx-dockmaker_vi.ts \
                translations/mx-dockmaker_xh.ts \
                translations/mx-dockmaker_yi.ts \
                translations/mx-dockmaker_yo.ts \
                translations/mx-dockmaker_yue_CN.ts \
                translations/mx-dockmaker_zh_CN.ts \
                translations/mx-dockmaker_zh_HK.ts \
                translations/mx-dockmaker_zh_TW.ts

RESOURCES += \
    images.qrc
