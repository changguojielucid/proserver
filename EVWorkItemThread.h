#pragma once

#include <QThread>
#include <QMap>

#include "EVWorkItem.h"


class EVWorkItemThread
{
public:
  EVWorkItemThread(bool bOneRenderWndMode = false);
  ~EVWorkItemThread();

  QThread* GetThread() { return m_pThread; }
  EVWorkItem* GetWorkItem() { return m_pWorkItem; }

private:
  QThread* m_pThread;
  EVWorkItem* m_pWorkItem;

};

