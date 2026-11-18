## Shared expansion-panel header.
##
## Cheetah's #include is a runtime include: it sees the search list and
## #set global vars, nothing else. Callers hand over three globals:
##
##   $panel_title  text shown in the header
##   $panel_id     id of the collapsible body, for data-target
##   $panel_type   'expanded', 'collapsed' or 'static'
##
## 'static' renders panel chrome with no toggle and no chevron, so Bootstrap
## never wires it up and the body stays open.
#set $ph_static = ($panel_type == 'static')
#set $ph_exp_s  = 'false' if $panel_type == 'collapsed' else 'true'
#if $ph_static
    <div class="nwm-panel-header nwm-panel-static">
        <span class="nwm-panel-title">$panel_title</span>
    </div>
#else
    <div class="nwm-panel-header" data-toggle="collapse"
         data-target="#$panel_id" aria-expanded="$ph_exp_s">
        <span class="nwm-panel-title">$panel_title</span>
        <span class="mdi mdi-chevron-down nwm-panel-icon"></span>
    </div>
#end if
