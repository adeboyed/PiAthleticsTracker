import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from rpc import GetStatus, StartRace

from django.shortcuts import render

# Create your views here.

from django.http import HttpResponse, JsonResponse


def index(request):
    return render(request, 'index.html', {})

import datetime
def get_time(request):
    status = GetStatus()
    time = status.get('last_race_time', -1)
    response = {'time': time, 'finished': status.get('race_in_progress', False)}
    return JsonResponse(response)



def get_status(request):
    status = GetStatus()
    Pis_connected = status.get('client_status', False)
    Gate_setup = status.get('light_gate_captured', False)
    response = {'PIs': Pis_connected, 'gate': Gate_setup}
    return JsonResponse(response)


def start_race(request):
    StartRace()
    return render(request, 'index.html', {'started_race': True})


