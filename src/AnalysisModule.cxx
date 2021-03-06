#include "SFrameTools/include/AnalysisModule.h"

#include <strings.h>

AnalysisModule::~AnalysisModule(){}
Context::~Context(){}
Hists::~Hists(){}
    
AndSelection::AndSelection(const identifier & selection_id, bool create_cutflow_histo): selid(selection_id),
    create_cutflow(create_cutflow_histo), cutflow_raw(0), cutflow_weighted(0){
}


void AndSelection::add(std::auto_ptr<SelectionModule> & module, const string & alternative_description){
    assert(module.get() != 0);
    if(alternative_description.empty()){
        descriptions.push_back(module->description());
    }
    else{
        descriptions.push_back(alternative_description);
    }
    modules.push_back(module.release());
    assert(module.get()==0);
}

    
void AndSelection::begin_dataset(Context & ctx){
    if(create_cutflow){
        string name = selid.name();
        cutflow_weighted = new TH1D(("cf_" + name).c_str(), ("Cutflow '" + name + "' using weights").c_str(), modules.size()+1, -1, modules.size());
        cutflow_raw = new TH1D(("cf_" + name + "_raw").c_str(), ("Cutflow '" + name + "' unweighted").c_str(), modules.size()+1, -1, modules.size());    
        TAxis * ax  = cutflow_raw->GetXaxis();
        ax->SetBinLabel(1, "all");
        for(size_t i=0; i<modules.size(); ++i){
            ax->SetBinLabel(i+2, descriptions[i].c_str());
        }
        ax = cutflow_weighted->GetXaxis();
        ax->SetBinLabel(1, "all");
        for(size_t i=0; i<modules.size(); ++i){
            ax->SetBinLabel(i+2, descriptions[i].c_str());
        }
        ctx.put(cutflow_raw->GetName(), cutflow_raw);
        ctx.put(cutflow_weighted->GetName(), cutflow_weighted);
    }
}

void AndSelection::process(EventCalc & event, Context & ctx){
    bool result = true;
    if(create_cutflow){
        assert(cutflow_raw != 0 and cutflow_weighted != 0);
        cutflow_raw->Fill(-1);
        cutflow_weighted->Fill(-1, event.GetWeight());
    }
    for(size_t i=0; i<modules.size(); ++i){
        if(modules[i].pass(event)){
            if(create_cutflow){
                cutflow_raw->Fill(i);
                cutflow_weighted->Fill(i, event.GetWeight());
            }
        }
        else{
            result = false;
            break;
        }
    }
    event.set_selection_passed(selid, result);
}


bool string2bool(const string & s){
    static const char * true_strings[] = {"true", "yes", "1", "on"};
    static const char * false_strings[] = {"false", "no", "0", "off"};
    for(size_t i=0; i<4; ++i){
        if(strcasecmp(s.c_str(), true_strings[i])==0) return true;
        if(strcasecmp(s.c_str(), false_strings[i])==0) return false;
    }
    throw std::runtime_error("could not interpret '" + s + "' as boolean");
}
