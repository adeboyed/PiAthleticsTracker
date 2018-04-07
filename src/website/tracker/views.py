from django.shortcuts import render

# Create your views here.

from django.http import HttpResponse, JsonResponse
from .rpc import GetStatus, StartRace


def index(request):
    return render(request, 'index.html', {})

import datetime
def get_time(request):
    status = GetStatus()
    time = status.get('lastRaceTime', -1)
    response = {'time': time, 'finished': status.get('raceInProgress', False)}
    return JsonResponse(response)



def get_status(request):
    status = GetStatus()
    Pis_connected = status.get('clientStatus', False)
    Gate_setup = status.get('lightGateCaptured', False)
    response = {'PIs': Pis_connected, 'gate': Gate_setup}
    return JsonResponse(response)


def start_race(request):
    StartRace()
    return render(request, 'index.html', {'started_race': True})


