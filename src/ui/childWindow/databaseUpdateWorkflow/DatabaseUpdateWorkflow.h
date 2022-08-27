//
// Created by Deamon on 8/27/2022.
//

#ifndef AWEBWOWVIEWERCPP_DATABASEUPDATEWORKFLOW_H
#define AWEBWOWVIEWERCPP_DATABASEUPDATEWORKFLOW_H


class DatabaseUpdateWorkflow {
public:
    DatabaseUpdateWorkflow();

    void render();
private:
    //Showing elements state.
    bool m_showDBDPrompt = true;



    bool m_needToUpdateDBD = false;

private:
    void defineDialogs();

};


#endif //AWEBWOWVIEWERCPP_DATABASEUPDATEWORKFLOW_H
